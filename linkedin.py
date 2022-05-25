from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver import ChromeOptions
import html2text
from selenium.common.exceptions import NoSuchElementException
import urllib.parse
import configparser
import random, time

import sqliteHelper

class Head_Hunter_9000:

    def __init__(self):
        config = configparser.ConfigParser()
        config.read('config.ini')

        self.username = config['LOGIN']['email']
        self.password = config['LOGIN']['password']

        self.redirect_url = self.make_url(config['SEARCH_FILTERS'])

        opts = ChromeOptions()
        opts.add_argument("--window-size=2560,1440")
        self.driver = webdriver.Chrome(executable_path = "./chromedriver", options = opts)

        self.litehelper = sqliteHelper.sqliteHelper(config['DATABASE'])

    def __del__(self):
        self.driver.close()

    def simulate_human_typing(self, html_input_el, desired_input):
        for letter in desired_input:
            time.sleep(random.uniform(0.1, 0.2))
            html_input_el.send_keys(letter)

    def login(self):
        url = "https://www.linkedin.com/login?emailAddress=&fromSignIn=&fromSignIn=true&session_redirect="
        url += urllib.parse.quote(self.redirect_url, safe='')
        self.driver.get(url)

        username_input = self.driver.find_element_by_css_selector('form.login__form input[name=session_key]')
        password_input = self.driver.find_element_by_css_selector('form.login__form input[name=session_password]')

        self.simulate_human_typing(username_input, self.username)
        self.simulate_human_typing(password_input, self.password)

        time.sleep(random.uniform(0.1, 0.5))
        submit_btn = self.driver.find_element_by_css_selector('form.login__form button[type=submit]')
        submit_btn.click()

    def scroll_through_sidebar(self):
        scroll_cnt = 0
        sidebar = self.driver.find_element_by_xpath("//div[@class='jobs-search-results display-flex flex-column']")
        while scroll_cnt < 5:
            self.driver.execute_script('arguments[0].scrollTop = arguments[0].scrollTop + arguments[0].offsetHeight;', sidebar)
            scroll_cnt += 1
            time.sleep(random.uniform(1, 3))

    def add_info_if_exists(self, job_dict, key, element, xpath_from_element):
        try:
            info = element.find_element_by_xpath(xpath_from_element).text.strip()
            job_dict[key] = info
        except NoSuchElementException:
            pass

    def build_job_info(self, ext_job_id, job_board='linkedin'):
        job_info = {}
        job_info_container = self.driver.find_element_by_xpath("//div[@class='job-view-layout jobs-details']")
        job_short = job_info_container.find_element_by_xpath(".//div[@class='jobs-unified-top-card__content--two-pane']")
        company_location_info = job_short.find_element_by_xpath(".//span[contains(@class, 'jobs-unified-top-card__subtitle-primary-grouping')]")
        self.add_info_if_exists(job_info, 'Salary', job_short, "(.//li[@class='jobs-unified-top-card__job-insight'])[1]")
        self.add_info_if_exists(job_info, 'NumEmployees', job_short, "(.//li[@class='jobs-unified-top-card__job-insight'])[2]")
        self.add_info_if_exists(job_info, 'Location', company_location_info, "./span[@class='jobs-unified-top-card__bullet']")
        self.add_info_if_exists(job_info, 'WorkplaceType', company_location_info, "./span[@class='jobs-unified-top-card__workplace-type']")
        job_info['JobTitle'] = job_short.find_element_by_xpath(".//h2").text.strip()
        job_info['CompanyName'] = company_location_info.find_element_by_xpath("./span[@class='jobs-unified-top-card__company-name']").text.strip()
        job_info['Description'] = html2text.html2text(job_info_container.find_element_by_xpath("//article//span").get_attribute("innerHTML"))
        job_info['JobBoardID'] = job_board
        job_info['AppSubmitted'] = 0
        job_info['ExtJobID'] = ext_job_id
        print(job_info)
        return job_info

    def submit_job_apps(self):
        time.sleep(random.uniform(1,2))
        self.scroll_through_sidebar()
        job_listings = self.driver.find_elements_by_xpath("//div[contains(@class, 'job-card-container') and contains(@class, 'job-card-list')]")
        
        for i in range(len(job_listings)):
            just_added = False
            time.sleep(random.uniform(1,2))
            link = job_listings[i].find_element_by_xpath(".//a[contains(@class, 'job-card-container__link') and contains(@class, 'job-card-list__title')]")
            link.click()
            ext_job_id = job_listings[i].get_attribute("data-job-id")
            if not self.litehelper.is_in_table('Jobs', **{'JobBoardID':'linkedin', 'ExtJobID': ext_job_id}):
                job_info = self.build_job_info(ext_job_id)
                self.litehelper.insert_job(**job_info)
                just_added = True

            # if just_added or self.litehelper.is_in_table('Jobs', **{'JobBoardID':'linkedin', 'ExtJobID': ext_job_id, 'AppSubmitted': 0}):
                # If all Questions for Job already have Answers

                # Else if there are not yet any Questions

                
            

    def make_url(self, search_filters):
        url_builder = 'https://www.linkedin.com/jobs/search/?f_AL=true&'

        exp_q = self.get_exp_q(int(search_filters['exp_level']))
        if exp_q:
            url_builder += "f_E=" + exp_q + "&"
       
        job_type_q = self.get_job_type_q(int(search_filters['job_type']))
        if job_type_q:
            url_builder += "f_JT=" + job_type_q + "&"

        date_posted_q = self.get_seconds_posted_ago_q(int(search_filters['date_posted']))
        if date_posted_q:
            url_builder += "f_TPR=r" + str(date_posted_q) + "&"

        on_site_remote_q = self.get_on_site_remote_q(int(search_filters['on_site_remote']))
        if on_site_remote_q:
            url_builder += "f_WT=" + str(on_site_remote_q) + "&"

        keywords_q = urllib.parse.quote(search_filters['job_title'])
        if keywords_q:
            url_builder += "keywords=" + keywords_q + "&"

        location_q = urllib.parse.quote(search_filters['location'])
        if location_q:
            url_builder += "location=" + location_q + "&"

        return url_builder[:-1]

    def get_on_site_remote_q(self, on_site_remote):
        if on_site_remote == 0:
            return None
        out = self.decode_num_to_bin(on_site_remote, 3)
        query = ''
        for i in range(len(out)):
            if out[i]:
                query += str(i+1) + ','

        return urllib.parse.quote(query[:-1])

    def get_job_type_q(self, job_type):
        bit_to_letter = {
            1: 'F',
            2: 'P',
            3: 'C',
            4: 'T',
            5: 'V',
            6: 'I',
            7: 'O'
        }
        if job_type == 0:
            return None
        out = self.decode_num_to_bin(job_type, 7)
        query = ''
        for i in range(len(out)):
            if out[i]:
                query += bit_to_letter[i+1] + ','
        return urllib.parse.quote(query[:-1])

    def decode_num_to_bin(self, value, num_bits):
        return [1 if value & (1 << (num_bits-1-n)) else 0 for n in range(num_bits-1,-1,-1)]
        
    def get_exp_q(self, exp_level):
        if exp_level == 0:
            return None
        out = self.decode_num_to_bin(exp_level, 6)
        query = ''
        for i in range(len(out)):
            if out[i]:
                query += str(i+1) + ','

        return urllib.parse.quote(query[:-1])
        

    def get_seconds_posted_ago_q(self, date_posted):
    
        if date_posted == 1: # any time
            return None
        elif date_posted == 2:
            seconds = 30*24*60*60
        elif date_posted == 3:
            seconds = 7*24*60*60
        elif date_posted == 4:
            seconds = 24*60*60
        else:
            seconds = max(date_posted, 24*60,60)
        
        return seconds


if __name__ == '__main__':
    hh_9000 = Head_Hunter_9000()
    hh_9000.login()
    hh_9000.submit_job_apps()