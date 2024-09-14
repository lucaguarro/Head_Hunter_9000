from selenium import webdriver
from selenium.webdriver.common.by import By
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.support.ui import Select
from selenium.webdriver import ChromeOptions
import html2text
from selenium.common.exceptions import NoSuchElementException
import urllib.parse
import configparser
import random
import time
import logging.config
logging.config.dictConfig({
    'version': 1,
    # Other configs ...
    'disable_existing_loggers': True
})
import logging

import re
import data.architecture as da
import data.manager as dm
import scraper.utils as utils
from exceptions import RegexParseError

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    filename="linkedin.log"
)

logger = logging.getLogger(__name__)

class Head_Hunter_9000:

    def __init__(self, config_file_path):
        config = configparser.ConfigParser()
        config.read(config_file_path)

        self.username = config['LOGIN']['email']
        self.password = config['LOGIN']['password']

        self.redirect_url = utils.make_url(config['SEARCH_FILTERS'])

        print(self.redirect_url)
        opts = ChromeOptions()
        opts.add_argument("--window-size=2560,1440")
        testing123 = ChromeDriverManager().install()
        testing123 = '/home/luca/Documents/Projects/Head_Hunter_9000/chromedriver-linux64/chromedriver'
        self.driver = webdriver.Chrome(testing123, options=opts)

    def __del__(self):
        self.driver.close()

    def login(self):
        url = "https://www.linkedin.com/login?emailAddress=&fromSignIn=&fromSignIn=true&session_redirect="
        url += urllib.parse.quote(self.redirect_url, safe='')
        self.driver.get(url)

        username_input = self.driver.find_element(By.CSS_SELECTOR, 'form.login__form input[name=session_key]')
        password_input = self.driver.find_element(By.CSS_SELECTOR, 'form.login__form input[name=session_password]')

        utils.simulate_human_typing(username_input, self.username)
        utils.simulate_human_typing(password_input, self.password)

        time.sleep(random.uniform(0.1, 0.5))
        submit_btn = self.driver.find_element(By.CSS_SELECTOR, 'form.login__form button[type=submit]')
        submit_btn.click()
        logger.debug("Logged into linkedin.")

    def scroll_through_sidebar(self):
        scroll_cnt = 0
        sidebar = self.driver.find_element(By.XPATH, "//div[@class='scaffold-layout__list ']/div[contains(@class, 'jobs-search-results-list')]")
        while scroll_cnt < 5:
            self.driver.execute_script('arguments[0].scrollTop = arguments[0].scrollTop + arguments[0].offsetHeight;', sidebar)
            scroll_cnt += 1
            time.sleep(random.uniform(1, 3))
        return sidebar

    def add_info_if_exists(self, job_dict, key, element, xpath_from_element):
        try:
            info = element.find_element(By.XPATH, xpath_from_element).text.strip()
            job_dict[key] = info
        except NoSuchElementException:
            pass

    def parse_sub_title_text(self, text):
        pattern = r'^(.*?) · (.*?) +(Reposted)? +(\d+ (?:hour|day|week|month)s? ago) +· +(\d+(?:,\d+)? applicants)$'
        match = re.match(pattern, text)

        if match:
            company_name = match.group(1)
            location = match.group(2)
            is_a_repost = True if match.group(3) == 'Reposted' else False
            posted_time_ago = match.group(4)
            num_applicants = match.group(5).replace(',', '')

            job_attributes = {
                "companyname": company_name,
                "location": location,
                "isarepost": is_a_repost,
                "postedtimeago": posted_time_ago,
                "numapplicants": num_applicants
            }
            return job_attributes
        else:
            raise RegexParseError(text, pattern)
        
    def get_salary_bounds(self, match):
        salary_lower_bound = match.group(1)
        salary_upper_bound = match.group(3)
        if salary_lower_bound:
            is_hourly = True if match.group(2) == '/hr' else False
            salary_lower_bound = utils.convert_to_integer(salary_lower_bound)
            salary_upper_bound = utils.convert_to_integer(salary_upper_bound)
            if is_hourly:
                salary_lower_bound = utils.convert_hourly_wage_to_yearly(salary_lower_bound)
                salary_upper_bound = utils.convert_hourly_wage_to_yearly(salary_upper_bound)
            return salary_lower_bound, salary_upper_bound
        else:
            return None, None
        
    def parse_first_line_text(self, text):
        pattern = r'^(?:\$(\d+(?:,\d+)?)(/hr|/yr)?\s*-\s*\$(\d+(?:,\d+)?)(?:/hr|/yr)?\s*)?(Hybrid|Remote|On-site)?\s*(Full-time|Part-time|Contract|Temp|Volunteer)?\s*(Internship|Entry level|Associate|Mid-Senior level|Director|Executive)?'
        match = re.match(pattern, text)

        if match:
            salary_lower_bound, salary_upper_bound = self.get_salary_bounds(match)
            onsite_remote = match.group(4)
            worktype = match.group(5)
            exp_level = match.group(6)

            job_attributes = {
                "salarylowerbound": salary_lower_bound,
                "salaryupperbound": salary_upper_bound,
                "workplacetype": onsite_remote,
                "jobtype": worktype,
                "explevel": exp_level
            }
            return job_attributes
        else:
            raise RegexParseError(text, pattern)

    def parse_second_line_text(self, text):
        pattern = r'^(\d+-\d+ employees)?(?: · )?(?:(.+))?$'
        match = re.search(pattern, text)

        if match:
            num_employees = match.group(1)
            industry = match.group(2)

            job_attributes = {
                "numemployees": num_employees,
                "industry": industry
            }
            return job_attributes
        else:
            raise RegexParseError(text, text)
    
    def build_job_info(self, job_info_container, ext_job_id, job_board='linkedin'):
        job_info = {}
        job_short = job_info_container.find_element(By.XPATH, ".//div[contains(@class, 'job-details-jobs-unified-top-card__container--two-pane')]")

        sub_title_text = job_short.find_element(By.XPATH, "./div[@class='job-details-jobs-unified-top-card__primary-description']").text
        job_info.update(self.parse_sub_title_text(sub_title_text))

        first_line_text = job_short.find_element(By.XPATH, "(.//li[@class='job-details-jobs-unified-top-card__job-insight'])[1]").text
        job_info.update(self.parse_first_line_text(first_line_text))

        second_line_text = job_short.find_element(By.XPATH, "(.//li[@class='job-details-jobs-unified-top-card__job-insight'])[2]").text
        job_info.update(self.parse_second_line_text(second_line_text))

        job_info['jobtitle'] = job_short.find_element(By.XPATH, ".//h2").text.strip()
        job_info['description'] = html2text.html2text(job_info_container.find_element(By.XPATH, "//article//span").get_attribute("innerHTML"))
        job_info['appsubmitted'] = True if job_short.find_elements(By.XPATH, "//span[@class='artdeco-inline-feedback__message']") else False
        job_info['extjobid'] = ext_job_id

        job_info['jobboardid'] = job_board
        return job_info

    def get_job_id(self, url):
        pattern = r'currentJobId=(\d+)'
        match = re.search(pattern, url)

        if match:
            current_job_id = match.group(1)
            return current_job_id
        else:
            print("Could not find job id.")
            return ''

    def scrape_freeresponse_questions(self, freeresponse_question_containers):
        questions = []
        for fr_q_c in freeresponse_question_containers:
            question_prompt = fr_q_c.find_element(By.TAG_NAME, "label").get_attribute("innerText")
            questions.append(question_prompt)

        return questions

    def scrape_dropdown_questions(self, dropdown_question_containers):
        questions_with_options = []

        for dd_q_c in dropdown_question_containers:
            question_prompt = dd_q_c.find_element(By.XPATH, "./label/span[@aria-hidden='true']").text

            select = dd_q_c.find_element(By.TAG_NAME, "select")
            options = select.find_elements(By.TAG_NAME, "option")

            # List to store the dictionaries
            option_list = []

            # Loop through the "option" elements and extract inner text and value
            for option in options:
                inner_text = option.text
                value = option.get_attribute("value")

                # Create a dictionary for each "option" element and add it to the list
                option_dict = {
                    "text": inner_text,
                    "value": value
                }
                option_list.append(option_dict)

            questions_with_options.append((question_prompt, option_list))

        return questions_with_options


    def scrape_radiobutton_questions(self, radiobutton_question_containers):
        questions_with_options = []

        for rb_q_c in radiobutton_question_containers:
            question_prompt = rb_q_c.find_element(By.XPATH, ".//span[@data-test-form-builder-radio-button-form-component__title]/span[@aria-hidden='true']").text
            print(question_prompt)

            input_containers = rb_q_c.find_elements(By.TAG_NAME, "div")

            # List to store the dictionaries
            option_list = []

            for input_container in input_containers:
                input = input_container.find_element(By.TAG_NAME, "input")
                value = input.get_attribute('value')

                inner_text = input_container.find_element(By.TAG_NAME, "label").text

                option_dict = {
                    "text": inner_text,
                    "value": value
                }
                option_list.append(option_dict)

            questions_with_options.append((question_prompt, option_list))
        
        return questions_with_options

    def fill_out_questions(self, freeresponse_question_containers, dropdown_question_containers, radiobutton_question_containers):
        for fr_q in freeresponse_question_containers:
            input_tag = fr_q.find_element(By.TAG_NAME, "input")
            input_tag.clear()
            input_tag.send_keys("1")

        for dd_q in dropdown_question_containers:
            select_el = dd_q.find_element(By.TAG_NAME, "select")
            select = Select(select_el)
            select.select_by_index(1)

        for rb_q in radiobutton_question_containers:
            radio_buttons = rb_q.find_elements(By.XPATH, ".//label")
            if radio_buttons:
                radio_buttons[0].click()

    def scrape_questions(self, job_info_container):
        # def get_rvw_btn():
        #     rvw_btn = hh_9000.driver.find_elements(By.XPATH, "//span[text()='Review']/ancestor::button")
        #     if rvw_btn:
        #         return rvw_btn[0]

        def get_next_btn():
            nxt_btn = self.driver.find_elements(By.XPATH, "//span[text()='Next']/ancestor::button")
            if nxt_btn:
                return nxt_btn[0]
            
            return None
        
        easy_apply_button = job_info_container.find_element(By.XPATH, "//button[contains(@class, 'jobs-apply-button')]")
        easy_apply_button.click()

        question_form = self.driver.find_element(By.XPATH, "//div[@class='pb4']")

        fr_prompts = []
        dd_prompts_and_options = []
        rb_prompts_and_options = []

        next_btn = True
        while next_btn:
            next_btn = get_next_btn()
            question_form = self.driver.find_element(By.XPATH, "//div[@class='pb4']")

            freeresponse_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//div[@data-test-single-line-text-form-component]")
            dropdown_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//div[@data-test-text-entity-list-form-component]")
            radiobutton_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//fieldset[@data-test-form-builder-radio-button-form-component]")

            self.fill_out_questions(freeresponse_question_containers, dropdown_question_containers, radiobutton_question_containers)

            fr_prompts.extend(self.scrape_freeresponse_questions(freeresponse_question_containers))
            dd_prompts_and_options.extend(self.scrape_dropdown_questions(dropdown_question_containers))
            rb_prompts_and_options.extend(self.scrape_radiobutton_questions(radiobutton_question_containers))

            if next_btn is not None:
                next_btn.click()

        all_questions = {'freeresponse': fr_prompts, 'dropdown': dd_prompts_and_options, 'radiobutton': rb_prompts_and_options}

        close_button = self.driver.find_element(By.XPATH, "//button[@data-test-modal-close-btn]")
        close_button.click()
        close_button2 = self.driver.find_element(By.XPATH, "//button[@data-test-dialog-secondary-btn]")
        close_button2.click()

        return all_questions
    
    def store_to_database(self, job_info, all_questions):
        jobboard_sa = dm.create_or_get_job_board('linkedin')

        questions_sa = []
        for fr_q in all_questions['freeresponse']:
            question_sa = dm.create_question(fr_q, da.QuestionType.FREERESPONSE, None)
            questions_sa.append(question_sa)

        for dd_q in all_questions['dropdown']:
            question_sa = dm.create_question_and_options(dd_q, da.QuestionType.DROPDOWN)
            questions_sa.append(question_sa)

        for rb_q in all_questions['radiobutton']:
            question_sa = dm.create_question_and_options(rb_q, da.QuestionType.RADIOBUTTON)
            questions_sa.append(question_sa)

        dm.create_job(job_info, jobboard_sa, questions_sa)
        dm.commit()

    def scan_job_apps(self, apply_mode_on=False):
        time.sleep(random.uniform(1, 2))
        jobs_sidebar = self.scroll_through_sidebar()
        job_listings = jobs_sidebar.find_elements(By.XPATH, "//div[contains(@class, 'job-card-container') and contains(@class, 'job-card-list')]")

        for i in range(len(job_listings)):
            just_added = False
            time.sleep(random.uniform(1, 2))
            link = job_listings[i].find_element(By.XPATH, ".//a[contains(@class, 'job-card-container__link') and contains(@class, 'job-card-list__title')]")
            link.click()
            time.sleep(random.uniform(1, 2))

            try:
                ext_job_id = self.get_job_id(self.driver.current_url)
                job_info_container = self.driver.find_element(By.XPATH, "//div[@class='job-view-layout jobs-details']")
                job_info = self.build_job_info(job_info_container, ext_job_id)
                if not job_info['appsubmitted']: # can also do a check here to see if job already exists in local db TODO
                    all_questions = self.scrape_questions(job_info_container)
                    self.store_to_database(job_info, all_questions)
            except RegexParseError as e:
                logger.error(e)

            # close job pop-up window
                
