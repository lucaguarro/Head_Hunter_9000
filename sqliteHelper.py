import sqlite3 as lite
import configparser
from urllib.request import pathname2url
from pathlib import Path


class sqliteHelper:

    def __init__(self, db_config):
        path = Path(db_config['db_path'])
        self.conn = None
        try:
            dburi = 'file:{}?mode=rw'.format(pathname2url(path))
            self.conn = lite.connect(dburi, uri=True)
        except Exception:
            print("oy")
            self.conn = lite.connect(path)

    def __del__(self):
        self.conn.close()

if __name__ == '__main__':

    config = configparser.ConfigParser()
    config.read('config.ini')

    litehelper = sqliteHelper(config['DATABASE'])