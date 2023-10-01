import time
import random
import urllib

def simulate_human_typing(html_input_el, desired_input):
    for letter in desired_input:
        time.sleep(random.uniform(0.1, 0.2))
        html_input_el.send_keys(letter)

def get_on_site_remote_q(on_site_remote):
    if on_site_remote == 0:
        return None
    out = decode_num_to_bin(on_site_remote, 3)
    query = ''
    for i in range(len(out)):
        if out[i]:
            query += str(i+1) + ','

    return urllib.parse.quote(query[:-1])

def get_job_type_q(job_type):
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
    out = decode_num_to_bin(job_type, 7)
    query = ''
    for i in range(len(out)):
        if out[i]:
            query += bit_to_letter[i+1] + ','
    return urllib.parse.quote(query[:-1])

def decode_num_to_bin(value, num_bits):
    return [1 if value & (1 << (num_bits-1-n)) else 0 for n in range(num_bits-1, -1, -1)]

def get_exp_q(exp_level):
    if exp_level == 0:
        return None
    out = decode_num_to_bin(exp_level, 6)
    query = ''
    for i in range(len(out)):
        if out[i]:
            query += str(i+1) + ','

    return urllib.parse.quote(query[:-1])

def get_seconds_posted_ago_q(date_posted):
    if date_posted == 1:  # any time
        return None
    elif date_posted == 2:
        seconds = 30*24*60*60
    elif date_posted == 3:
        seconds = 7*24*60*60
    elif date_posted == 4:
        seconds = 24*60*60
    else:
        seconds = max(date_posted, 24*60, 60)
    return seconds

# TODO this should be rewritten to use ''.join so as to not constantly allocate memory for each time we append to the string
def make_url(search_filters):
    url_builder = 'https://www.linkedin.com/jobs/search/?f_AL=true&'

    exp_q = get_exp_q(int(search_filters['exp_level']))
    if exp_q:
        url_builder += "f_E=" + exp_q + "&"
    job_type_q = get_job_type_q(int(search_filters['job_type']))
    if job_type_q:
        url_builder += "f_JT=" + job_type_q + "&"

    date_posted_q = get_seconds_posted_ago_q(int(search_filters['date_posted']))
    if date_posted_q:
        url_builder += "f_TPR=r" + str(date_posted_q) + "&"

    on_site_remote_q = get_on_site_remote_q(int(search_filters['on_site_remote']))
    if on_site_remote_q:
        url_builder += "f_WT=" + str(on_site_remote_q) + "&"

    keywords_q = urllib.parse.quote(search_filters['job_title'])
    if keywords_q:
        url_builder += "keywords=" + keywords_q + "&"

    location_q = urllib.parse.quote(search_filters['location'])
    if location_q:
        url_builder += "location=" + location_q + "&"

    return url_builder[:-1]

def convert_to_integer(number_string):
    # Remove commas from the number string
    cleaned_string = number_string.replace(',', '')
    
    try:
        # Convert the cleaned string to an integer
        integer_value = int(cleaned_string)
        return integer_value
    except ValueError:
        # Handle the case where the input is not a valid number
        print("Error: Invalid number format")
        return None
    
def convert_hourly_wage_to_yearly(hourly_wage):
    return hourly_wage * 40 * 52.1429 # 40 hrs per week, 52.1429 weeks in a year