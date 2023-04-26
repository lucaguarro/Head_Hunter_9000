import logging
import pathlib

def makeLogger(name):
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)

    formatter = logging.Formatter('%(asctime)s:%(name)s:%(message)s')

    custom_log_path = pathlib.Path().absolute() / "Logs" / (name + ".log")
    file_handler = logging.FileHandler(custom_log_path, 'a', 'utf-8')
    file_handler.setFormatter(formatter)

    logger.addHandler(file_handler)
    return logger