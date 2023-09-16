import scraper.linkedin as li

if __name__ == '__main__':
    hh_9000 = li.Head_Hunter_9000('config.ini')
    hh_9000.login()
    hh_9000.submit_job_apps()