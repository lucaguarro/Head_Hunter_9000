from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.chrome.options import Options
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

        self.driver = webdriver.Chrome(executable_path="./chromedriver")

    # def __del__(self):
        # self.driver.close()

    def simulate_human_typing(self, html_input_el, desired_input):
        for letter in desired_input:
            time.sleep(random.uniform(0.1, 0.5))
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

        print(url)

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



hh_9000 = Head_Hunter_9000()
hh_9000.login()