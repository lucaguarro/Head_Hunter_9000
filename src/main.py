import argparse
import scraper.linkedin as li
import configparser
import signal
import sys

def signal_handler(sig, frame):
    print('Python script received termination signal. Exiting gracefully.')
    sys.exit(0)

signal.signal(signal.SIGTERM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)  # Handle Ctrl+C as well

def main(config_path):
    config = configparser.ConfigParser()
    read_files = config.read(config_path)
    if not read_files:
        print(f"Error: Could not read config file at {config_path}")
        sys.exit(1)

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

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Head Hunter 9000 Script')
    parser.add_argument(
        '-c', '--config',
        type=str,
        required=True,
        help='Path to the configuration .ini file'
    )
    args = parser.parse_args()
    main(args.config)
