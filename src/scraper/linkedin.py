from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import Select
from selenium.webdriver import ChromeOptions
import html2text
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import TimeoutException
import urllib.parse
import random
import time
import logging.config
logging.config.dictConfig({
    'version': 1,
    # Other configs ...
    'disable_existing_loggers': True
})
import logging
import requests

import scraper.xpaths as xpaths
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

    def __init__(self, config):
        self.config = config

        self.redirect_url = utils.make_url(config['SEARCH_FILTERS'])

        print(self.redirect_url)
        opts = ChromeOptions()
        opts.add_argument("--window-size=2560,1440")

        # Debugger detection
        self.debugger_port = 9222  # Change this if you use a different port
        chromedriver_path = config['SCRAPER']['chromedriver_filepath']
        
        self.debugger_running = None
        if self.is_debugger_running():
            # Connect to existing Chrome session
            print("Debugger detected. Connecting to existing Chrome session...")
            opts.add_experimental_option("debuggerAddress", f"localhost:{self.debugger_port}")
        else:
            # Start a new Chrome instance
            print("No debugger detected. Starting a new Chrome session...")

        da.initialize_engine(config['DATABASE']['db_filepath'])
        dm.initialize_session()
        self.driver = webdriver.Chrome(executable_path=chromedriver_path, options=opts)

    def is_debugger_running(self):
        """
        Check if Chrome debugger is running by trying to connect to the debugging port.
        """
        if not self.debugger_running:
            try:
                response = requests.get(f"http://localhost:{self.debugger_port}/json")
                self.debugger_running = True
                return response.status_code == 200
            except requests.ConnectionError:
                self.debugger_running = False
                return False
        else:
            return self.debugger_running

    def __del__(self):
        self.driver.close()

    def login(self):
        url = "https://www.linkedin.com/login?emailAddress=&fromSignIn=&fromSignIn=true&session_redirect="
        url += urllib.parse.quote(self.redirect_url, safe='')
        self.driver.get(url)

        login_page_xpaths = xpaths.root_node.login_main

        username_input = self.driver.find_element(By.CSS_SELECTOR, login_page_xpaths.login_username_input.xpath)
        password_input = self.driver.find_element(By.CSS_SELECTOR, login_page_xpaths.login_password_input.xpath)

        utils.simulate_human_typing(username_input, self.config['LOGIN']['email'])
        utils.simulate_human_typing(password_input, self.config['LOGIN']['password'])

        time.sleep(random.uniform(0.1, 0.5))
        submit_btn = self.driver.find_element(By.CSS_SELECTOR, login_page_xpaths.login_submit_button.xpath)
        submit_btn.click()
        logger.debug("Logged into linkedin.")

    def scroll_through_sidebar(self, jobapps_sidebar_xpath):
        scroll_cnt = 0
        sidebar = self.driver.find_element(By.XPATH, jobapps_sidebar_xpath.xpath)
        if self.is_debugger_running() and not self.config.getboolean('DEBUGGER', 'skip_sidebar_scroll'): # skip this to save time during debugging
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

    def parse_sub_title_text(self, text, pattern):
        match = re.match(pattern, text)
        
        if match:
            location = match.group(1)  # Capture location
            posted_time_ago = match.group(3)  # Capture posted time (group 3 now, due to optional "Reposted")
            num_applicants = match.group(4).replace(',', '')  # Capture number of applicants, remove commas

            job_attributes = {
                "location": location,
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

    def parse_salary_range(self, salary_text):
        # Check if the salary is hourly or yearly
        is_hourly = "/hr" in salary_text.lower()

        # Extract the numeric values using a regex pattern that captures the salary range
        salary_pattern = r"\$([\d,.]+)[kKmM]?/?.*\$([\d,.]+)[kKmM]?/?.*"
        match = re.search(salary_pattern, salary_text)

        if match:
            lower_bound = match.group(1)
            upper_bound = match.group(2)

            # Convert to numeric (handle 'k' for thousand)
            lower_bound = float(lower_bound.replace(',', ''))
            upper_bound = float(upper_bound.replace(',', ''))

            # Convert to annual if it's hourly
            if is_hourly:
                lower_bound *= 2080  # 2080 hours per year for a 40 hr/week job
                upper_bound *= 2080
            else:
                # If 'k' is present in yearly salaries, multiply by 1000
                lower_bound *= 1000 if 'k' in salary_text.lower() else 1
                upper_bound *= 1000 if 'k' in salary_text.lower() else 1

            return lower_bound, upper_bound

        return None, None  # If parsing fails, return None


    def parse_first_line_elements(self, elements):
        job_attributes = {}
        for element in elements:
            text = element.text.lower()  # Convert text to lowercase for case-insensitive matching

            if "$" in text:  # Check if the element contains a dollar sign
                lower, upper = self.parse_salary_range(text)
                if lower and upper:
                    job_attributes['salarylowerbound'] = lower
                    job_attributes['salaryupperbound'] = upper
            
            elif "level" in text:  # Check if the element contains the word "level"
                job_attributes['explevel'] = text
            
            elif "time" in text:  # Check if the element contains the substring "time"
                job_attributes['jobtype'] = text

            elif text == 'onsite' or text == 'hybrid' or text == 'remote':
                job_attributes['workplacetype'] = text

        return job_attributes

        
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
        job_info_container_xpaths = xpaths.root_node.jobapps_main.jobinfo_container
        job_short_xpaths = job_info_container_xpaths.job_short

        job_info = {}
        job_short = job_info_container.find_element(By.XPATH, job_short_xpaths.xpath)

        sub_title_text = job_short.find_element(By.XPATH, job_short_xpaths.subtitle.xpath).text
        job_info.update(self.parse_sub_title_text(sub_title_text, job_short_xpaths.subtitle.regex))

        first_line_elements = job_short.find_elements(By.XPATH, job_short_xpaths.firstline.xpath)
        job_info.update(self.parse_first_line_elements(first_line_elements))
        # first_line_text = " ".join(element.text for element in job_short.find_elements(By.XPATH, job_short_xpaths.firstline.xpath))
        # job_info.update(self.parse_first_line_text(first_line_text))

        # second_line_text = job_short.find_element(By.XPATH, job_short_xpaths.secondline.xpath).text
        # job_info.update(self.parse_second_line_text(second_line_text))

        job_info['companyname'] = job_short.find_element(By.XPATH, job_short_xpaths.companyname.xpath).text.strip()
        job_info['jobtitle'] = job_short.find_element(By.XPATH, job_short_xpaths.jobtitle.xpath).text.strip()
        job_info['description'] = job_info_container.find_element(By.XPATH, job_info_container_xpaths.description.xpath).text
        job_info['appsubmitted'] = True if job_short.find_elements(By.XPATH, job_short_xpaths.appsubmitted.xpath) else False
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

    def scrape_freeresponse_questions(self, freeresponse_question_containers, freeresponse_question_container_xpaths):
        questions = []
        for fr_q_c in freeresponse_question_containers:
            try:
                # Try to find the label first
                question_prompt = fr_q_c.find_element(By.TAG_NAME, "label").get_attribute("innerText")
            except NoSuchElementException:
                # If no label is found, check for "Screener Question"
                try:
                    # Navigate up the DOM and find the spans related to the screener question
                    # First span (group title)
                    group_title = fr_q_c.find_element(By.XPATH, freeresponse_question_container_xpaths.screener_question_title.xpath).get_attribute("innerText")
                    
                    # Second span (group subtitle)
                    group_subtitle = fr_q_c.find_element(By.XPATH, freeresponse_question_container_xpaths.screener_question_subtitle.xpath).get_attribute("innerText")
                    
                    # Combine the title and subtitle as the question prompt
                    question_prompt = f"{group_title}: {group_subtitle}"
                except NoSuchElementException:
                    # If the spans are not found either, continue with the next element
                    continue

            is_multiline = False
            multiline_entity = fr_q_c.get_attribute("data-test-multiline-text-form-component")
            if multiline_entity is not None:
                is_multiline = True
            
            # Append the question prompt (either from label or screener question) to the list
            questions.append((question_prompt, is_multiline))

        return questions

    def scrape_dropdown_questions(self, dropdown_question_containers, dropdown_question_container_xpaths):
        questions_with_options = []

        for dd_q_c in dropdown_question_containers:
            try:
                question_prompt = dd_q_c.find_element(By.XPATH, "./label/span[@aria-hidden='true']").text
            except NoSuchElementException:
                question_prompt = dd_q_c.find_element(By.XPATH, dropdown_question_container_xpaths.app_aware_question_title.xpath).get_attribute("innerText")

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
    
    def scrape_checkbox_questions(self, checkbox_question_containers):
        questions_with_options = []

        for cb_q_c in checkbox_question_containers:
            question_prompt = cb_q_c.find_element(By.XPATH, ".//span[@data-test-form-builder-checkbox-form-component__title]/span[@aria-hidden='true']").text
            print(question_prompt)

            input_containers = cb_q_c.find_elements(By.TAG_NAME, "div")

            # List to store the dictionaries
            option_list = []

            for input_container in input_containers:
                input = input_container.find_element(By.TAG_NAME, "input")
                value = input.get_attribute('data-test-text-selectable-option__input')

                inner_text = input_container.find_element(By.TAG_NAME, "label").text

                option_dict = {
                    "text": inner_text,
                    "value": value
                }
                option_list.append(option_dict)

            questions_with_options.append((question_prompt, option_list))
        
        return questions_with_options

    def fill_out_questions(self, freeresponse_question_containers, dropdown_question_containers, radiobutton_question_containers, freeresponse_question_container_xpaths):
        for fr_q in freeresponse_question_containers:
            input_or_textarea = fr_q.find_element(By.XPATH, ".//input | .//textarea")
            # input_tag = fr_q.find_element(By.TAG_NAME, "input")
            input_or_textarea.clear()
            input_or_textarea.send_keys("1")
            typeahead_entity = fr_q.get_attribute("data-test-single-typeahead-entity-form-component")
            if typeahead_entity is not None:
                try:
                    first_option = WebDriverWait(fr_q, 10).until(
                        EC.presence_of_element_located((By.XPATH, freeresponse_question_container_xpaths.type_ahead_dropdown_first_option.xpath))
                    )
                except TimeoutException:
                    print("Element was not found within the timeout period.")
                first_option.click()

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
        jobapps_main_xpaths = xpaths.root_node.jobapps_main
        jobapp_popup_xpaths = jobapps_main_xpaths.jobapp_popup
        questionform_xpaths = jobapp_popup_xpaths.question_form
        
        easy_apply_button = job_info_container.find_element(By.XPATH, jobapps_main_xpaths.jobinfo_container.easyapply_button.xpath)
        easy_apply_button.click()

        jobapp_popup = self.driver.find_element(By.XPATH, jobapp_popup_xpaths.xpath)
        def get_next_btn():
            nxt_btn = jobapp_popup.find_elements(By.XPATH, jobapp_popup_xpaths.nextpage_button.xpath)
            if nxt_btn:
                return nxt_btn[0]
            
            return None
        
        def is_there_workexperience_form():
            we_form = jobapp_popup.find_elements(By.XPATH, jobapp_popup_xpaths.workexperience_form.xpath)
            if we_form:
                return True
            else:
                return False

        fr_prompts = []
        dd_prompts_and_options = []
        rb_prompts_and_options = []
        cb_prompts_and_options = []

        next_btn = True
        while next_btn:
            next_btn = get_next_btn()

            if not is_there_workexperience_form():
                question_form = jobapp_popup.find_element(By.XPATH, questionform_xpaths.xpath)

                freeresponse_question_containers = question_form.find_elements(By.XPATH, questionform_xpaths.freeresponse_question_container.xpath)
                dropdown_question_containers = question_form.find_elements(By.XPATH, questionform_xpaths.dropdown_question_container.xpath)
                radiobutton_question_containers = question_form.find_elements(By.XPATH, questionform_xpaths.radiobutton_question_container.xpath)
                checkbox_question_containers = question_form.find_elements(By.XPATH, questionform_xpaths.checkbox_question_container.xpath)

                self.fill_out_questions(freeresponse_question_containers, dropdown_question_containers, radiobutton_question_containers, questionform_xpaths.freeresponse_question_container)

                fr_prompts.extend(self.scrape_freeresponse_questions(freeresponse_question_containers, questionform_xpaths.freeresponse_question_container))
                dd_prompts_and_options.extend(self.scrape_dropdown_questions(dropdown_question_containers, questionform_xpaths.dropdown_question_container))
                rb_prompts_and_options.extend(self.scrape_radiobutton_questions(radiobutton_question_containers))
                cb_prompts_and_options.extend(self.scrape_checkbox_questions(checkbox_question_containers))

            if next_btn is not None:
                next_btn.click()

        all_questions = {'freeresponse': fr_prompts, 'dropdown': dd_prompts_and_options, 'radiobutton': rb_prompts_and_options, 'checkbox': cb_prompts_and_options}

        close_button = self.driver.find_element(By.XPATH, jobapp_popup_xpaths.closepage_button.xpath)
        close_button.click()
        close_button2 = self.driver.find_element(By.XPATH, jobapp_popup_xpaths.closepage_button2.xpath)
        close_button2.click()

        return all_questions
    
    def store_to_database(self, job_info, all_questions):
        jobboard_sa = dm.create_or_get_job_board('linkedin')

        questions_sa = []
        for fr_q in all_questions['freeresponse']:
            question_sa = dm.create_question(fr_q[0], da.QuestionType.FREERESPONSE, None, fr_q[1])
            questions_sa.append(question_sa)

        for dd_q in all_questions['dropdown']:
            question_sa = dm.create_question_and_options(dd_q, da.QuestionType.DROPDOWN)
            questions_sa.append(question_sa)

        for rb_q in all_questions['radiobutton']:
            question_sa = dm.create_question_and_options(rb_q, da.QuestionType.RADIOBUTTON)
            questions_sa.append(question_sa)

        # TODO add checkbox questions and add to database

        dm.create_job(job_info, jobboard_sa, questions_sa)
        dm.commit()

    def find_next_page_button(self, jobs_sidebar, curr_page_number, jobapps_sidebar_xpaths):
        next_page_button_xpath = jobapps_sidebar_xpaths.next_page_button.xpath.format(pagenumber=curr_page_number)
        try:
            return jobs_sidebar.find_element(By.XPATH, next_page_button_xpath)
        except NoSuchElementException:
            return None
        
    def process_job(self):
        try:
            ext_job_id = self.get_job_id(self.driver.current_url)
            job_info_container = self.driver.find_element(By.XPATH, xpaths.root_node.jobapps_main.jobinfo_container.xpath)
            job_info = self.build_job_info(job_info_container, ext_job_id)
            if not job_info['appsubmitted']: # can also do a check here to see if job already exists in local db TODO
                all_questions = self.scrape_questions(job_info_container)
                self.store_to_database(job_info, all_questions)
        except RegexParseError as e:
            logger.error(e)

    def scan_job_apps(self, apply_mode_on=False):
        time.sleep(random.uniform(1, 2))
        jobapps_sidebar_xpaths = xpaths.root_node.jobapps_main.jobapps_sidebar
        jobs_sidebar = self.scroll_through_sidebar(jobapps_sidebar_xpaths)

        curr_page_number = 1
        next_page_button = self.find_next_page_button(jobs_sidebar, curr_page_number, jobapps_sidebar_xpaths)

        while(next_page_button):
            next_page_button.click()
            job_listings = jobs_sidebar.find_elements(By.XPATH, jobapps_sidebar_xpaths.sidebar_listings.xpath)

            for i in range(len(job_listings)):
                time.sleep(random.uniform(1, 2))
                link = job_listings[i].find_element(By.XPATH, jobapps_sidebar_xpaths.sidebar_listings.listing_link.xpath)
                link.click()
                time.sleep(random.uniform(1, 2))

                self.process_job()

            curr_page_number += 1
            next_page_button = self.find_next_page_button(jobs_sidebar, curr_page_number, jobapps_sidebar_xpaths)
                
