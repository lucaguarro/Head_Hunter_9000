import scraper.linkedin as li

if __name__ == '__main__':
    hh_9000 = li.Head_Hunter_9000('config.ini')
    if not hh_9000.logged_in:
        hh_9000.login()
    hh_9000.scan_job_apps(False)