import scraper.linkedin as li
import configparser

if __name__ == '__main__':
    config = configparser.ConfigParser()
    config.read('config.ini')
    hh_9000 = li.Head_Hunter_9000(config)

    if hh_9000.is_debugger_running():
        debugger_config = config['DEBUGGER']
        skip_to = debugger_config.get('skip_to', fallback='')
        if skip_to == 'process_job':
            hh_9000.process_job()
        elif debugger_config.getboolean('skip_login'):
            hh_9000.scan_job_apps(False)
    else:
        hh_9000.login()
        hh_9000.scan_job_apps(False)