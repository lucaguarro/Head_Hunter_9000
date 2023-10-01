class RegexParseError(Exception):

    def __init__(self, job_string, pattern):
        self.job_string = job_string
        self.pattern = pattern

    def __str__(self) -> str:
        return f"""Could not parse job information from the following string: \"{self.job_string}\"
                   Using the following regex pattern: \"{self.pattern}\""""