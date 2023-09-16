from selenium import webdriver
from selenium.webdriver.common.by import By
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.common.keys import Keys
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
import scraper.utils as utils
from exceptions import ParseJobError

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
        self.driver = webdriver.Chrome(ChromeDriverManager().install(), options=opts)

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

    def add_info_if_exists(self, job_dict, key, element, xpath_from_element):
        try:
            info = element.find_element(By.XPATH, xpath_from_element).text.strip()
            job_dict[key] = info
        except NoSuchElementException:
            pass

    def parse_job_string(self, job_string):
        pattern = r'^(.*?) · (.*?) \((.*?)\) +(\d+ (day|week|month)s? ago) +· +(\d+(?:,\d+)? applicants)$'
        match = re.match(pattern, job_string)

        if match:
            company_name = match.group(1)
            location = match.group(2)
            workplace_type = match.group(3)
            posted_time_ago = match.group(4)
            num_applicants = match.group(6).replace(',', '')

            job_attributes = {
                "companyname": company_name,
                "location": location,
                "workplacetype": workplace_type,
                "postedtimeago": posted_time_ago,
                "numapplicants": num_applicants
            }
            return job_attributes
        else:
            raise ParseJobError(job_string, pattern)
    
    def build_job_info(self, job_info_container, ext_job_id, job_board='linkedin'):
        job_info = {}
        job_short = job_info_container.find_element(By.XPATH, ".//div[@class='jobs-unified-top-card__content--two-pane']")
        company_location_info = job_short.find_element(By.XPATH, "./div[@class='jobs-unified-top-card__primary-description']")
        job_info.update(self.parse_job_string(company_location_info.text))

        self.add_info_if_exists(job_info, 'salary', job_short, "(.//li[@class='jobs-unified-top-card__job-insight'])[1]")
        self.add_info_if_exists(job_info, 'numemployees', job_short, "(.//li[@class='jobs-unified-top-card__job-insight'])[2]")
        job_info['jobtitle'] = job_short.find_element(By.XPATH, ".//h2").text.strip()
        job_info['description'] = html2text.html2text(job_info_container.find_element(By.XPATH, "//article//span").get_attribute("innerHTML"))
        job_info['jobboardid'] = job_board
        job_info['appsubmitted'] = True if job_short.find_elements(By.XPATH, "//span[@class='artdeco-inline-feedback__message']") else False
        job_info['extjobid'] = ext_job_id
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
            question_prompt = fr_q_c.find_element(By.TAG_NAME, "label")
            # fr_q_c.find_element(By.TAG_NAME, "input") <-- get input field
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
            question_prompt = rb_q_c.find_element(By.XPATH, "//span[@data-test-form-builder-radio-button-form-component__title]/span[@aria-hidden='true']").text
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


    def scrape_questions(self, job_info_container):
        def get_next_btn():
            nxt_btn = self.driver.find_elements(By.XPATH, "//span[text()='Next']/ancestor::button")
            if not nxt_btn:
                nxt_btn = self.driver.find_elements(By.XPATH, "//span[text()='Review']/ancestor::button")
            
            if not nxt_btn:
                return None
            else:
                return nxt_btn[0]


        easy_apply_button = job_info_container.find_element(By.XPATH, "//button[contains(@class, 'jobs-apply-button')]")
        easy_apply_button.click()

        question_form = self.driver.find_element(By.XPATH, "//div[@class='pb4']")

        freeresponse_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//div[@data-test-single-line-text-form-component]")
        dropdown_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//div[@data-test-text-entity-list-form-component]")
        radiobutton_question_containers = question_form.find_elements(By.XPATH, "./div[contains(@class, 'jobs-easy-apply-form-section__grouping')]//fieldset[@data-test-form-builder-radio-button-form-component]")

        next_btn = get_next_btn()
        while next_btn:

            fr_prompts = self.scrape_freeresponse_questions(freeresponse_question_containers)
            dd_prompts_and_options = self.scrape_dropdown_questions(dropdown_question_containers)
            rb_prompts_and_options = self.scrape_radiobutton_questions(radiobutton_question_containers)

            next_btn.click()
            next_btn = get_next_btn()

        # //div[@class='pb4']
        # A - Gets the question containers //div[@class='pb4']/div[contains(@class, 'jobs-easy-apply-form-section__grouping')]
        # B - Gets text question container starting from A  //div[@data-test-single-line-text-form-component]
        # C - Gets radio button questions starting from A //fieldset[@data-test-form-builder-radio-button-form-component]
        # //span[text()='Next']/ancestor::button
        # //li-icon[@type='cancel-icon']/ancestor::button
        all_questions = {'freeresponse': fr_prompts, 'dropdown': dd_prompts_and_options, 'radiobutton': rb_prompts_and_options}
        return all_questions

    def submit_job_apps(self):
        time.sleep(random.uniform(1, 2))
        self.scroll_through_sidebar()
        job_listings = self.driver.find_elements(By.XPATH, "//div[contains(@class, 'job-card-container') and contains(@class, 'job-card-list')]")

        for i in range(len(job_listings)):
            just_added = False
            time.sleep(random.uniform(1, 2))
            link = job_listings[i].find_element(By.XPATH, ".//a[contains(@class, 'job-card-container__link') and contains(@class, 'job-card-list__title')]")
            link.click()

            try:
                ext_job_id = self.get_job_id(self.driver.current_url)
                job_info_container = self.driver.find_element(By.XPATH, "//div[@class='job-view-layout jobs-details']")
                job_info = self.build_job_info(job_info_container, ext_job_id)
                if not job_info['appsubmitted']:
                    all_questions = self.scrape_questions(job_info_container)
            except ParseJobError as e:
                logger.error(e)
                
