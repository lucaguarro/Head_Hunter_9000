import sqlite3 as lite
import configparser
from urllib.request import pathname2url
from pathlib import Path
import os.path
import utils
import os
import sys
from enum import Enum

class PathCode(Enum):
    VALID_SQL_FILE = 1
    NO_FILE_EXISTS = 2
    INVALID_SQL_FILE = 3

class sqliteHelper:

    # Init and Start-up Functions

    def __init__(self, db_config):
        path = Path(db_config['db_filename'])
        self.logger = utils.makeLogger("sqlite")

        if self.isSQLite3(path) == PathCode.VALID_SQL_FILE:
            self.logger.info("Using database at path: {} / {}".format(os.getcwd(), path))
            self.connect_sql(path)

        elif self.isSQLite3(path) == PathCode.NO_FILE_EXISTS:
            self.logger.info("Creating and using database at path: {} / {}".format(os.getcwd(), path))
            self.connect_sql(path)
            self.create_db_schema()
            self.configure_db()

        else:
            self.logger.error("Invalid SQLite database")
            sys.exit(1)

        self.setup()

    def setup(self):
        c = self.conn.cursor()
        self.job_boards_dict = {}
        for row in c.execute("SELECT JobBoardName, JobBoardID FROM JobBoards"):
            self.job_boards_dict[row[0]] = row[1]
            

    def connect_sql(self, path):
        try:
            self.conn = lite.connect(path)
        except lite.OperationalError:
            self.logger.error("UNABLE TO CREATE DB CONNECTION AT PATH {}".format(path))
            self.logger.error("SHUTTING DOWN APPLICATION")
            sys.exit(1)


    def isSQLite3(self, filename):

        if not os.path.isfile(filename):
            return PathCode.NO_FILE_EXISTS
        if os.path.getsize(filename) < 100: # SQLite database file header is 100 bytes
            return PathCode.INVALID_SQL_FILE

        with open(filename, 'rb') as fd:
            header = fd.read(100)

        if header[0:16] == b'SQLite format 3\000':
            return PathCode.VALID_SQL_FILE
        else:
            return PathCode.INVALID_SQL_FILE

    '''
    Known issues: need to check for missing tables as well
    '''
    def check_db_schema(self):
        self.logger.info("Checking sqlite schema")
        try:
            with open("./Database/tables_creation.txt", "r") as db_schema:
                sql_qs = db_schema.read().split('\n\n')
                sql_qs_dict = {}
                for sql_q in sql_qs:
                    key_val = sql_q.split(':::')
                    sql_qs_dict[key_val[0].strip()] = key_val[1].strip()
        except:
            self.logger.warning("Unable to open sqlite schema file")

        c = self.conn.cursor()

        for (table_name, query) in sql_qs_dict.items():
            c.execute("select sql from sqlite_master where type = 'table' and tbl_name = '{}'".format(table_name))
            r = c.fetchone()[0]
            if r != query:
                self.logger.warning("SQLite schema check failed")
                return False
                
        self.logger.info("SQLite schema check passed")
        return True


    def create_db_schema(self):
        self.logger.info("Attempting to create sqlite schema")
        last_q = ''
        try:
            with open("./Database_Scripts/tables_creation.txt", "r") as db_schema:
                sql_qs = db_schema.read().split('\n\n')
                sql_qs_dict = {}
                for sql_q in sql_qs:
                    key_val = sql_q.split(':::')
                    sql_qs_dict[key_val[0]] = key_val[1]

            c = self.conn.cursor()
            for sql_q in sql_qs_dict.values():
                last_q = sql_q
                self.logger.info("Running query: {}".format(sql_q))
                c.execute(sql_q)
  
            self.logger.info("Schema created successfully")
        except Exception:
            self.logger.error("Something went wrong when trying to create the database schema")
            self.logger.error("Last query attempted: {}".format(last_q))
            self.logger.error("SHUTTING DOWN APPLICATION")
            raise

    def configure_db(self):
        self.logger.info("Attempting to configure sqlite database")
        try:
            with open ("./Database_Scripts/configure_db.sql", "r") as db_config:
                c = self.conn.cursor()
                c.executescript(db_config.read())
        except Exception:
            self.logger.error("Something went wrong when trying to run the script in ./Database_Scripts/configure_db.sql")
            self.logger.error("SHUTTING DOWN APPLICATION")

    # DB insert functions

    def insert_job(self, job_desc):
        pass

    def __del__(self):
        self.conn.close()

if __name__ == '__main__':

    config = configparser.ConfigParser()
    config.read('config.ini')

    litehelper = sqliteHelper(config['DATABASE'])

    # print(lines)
    # print(len(lines))