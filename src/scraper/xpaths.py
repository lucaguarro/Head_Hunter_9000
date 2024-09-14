xpaths = {
    "login-main": {
        "login-username-input": "form.login__form input[name=session_key]",
        "login-password-input": "form.login__form input[name=session_password]",
        "login-submit-button": "form.login__form button[type=submit]"
    },
    "jobapps-main": {
        "jobapps-sidebar": "//div[@class='scaffold-layout__list ']/div[contains(@class, 'jobs-search-results-list')]",
        "sidebar-listings": "//div[contains(@class, 'job-card-container') and contains(@class, 'job-card-list')]",
        "listing-link": ".//a[contains(@class, 'job-card-container__link') and contains(@class, 'job-card-list__title')]",
        "jobinfo-container": {
            "job-short": ".//div[contains(@class, 'job-details-jobs-unified-top-card__container--two-pane')]",
            "subtitle-text": ".//div[@class='job-details-jobs-unified-top-card__primary-description-container']",
            "firstline-text": "(.//li[@class='job-details-jobs-unified-top-card__job-insight'])[1]",
            "secondline-text": "(.//li[@class='job-details-jobs-unified-top-card__job-insight'])[2]",
            "jobtitle": ".//h1",
            "description": "//article//span",
            "appsubmitted": "//span[@class='artdeco-inline-feedback__message']"
        }
    }
}