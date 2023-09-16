import architecture as a
from sqlalchemy import select, and_, or_

def insert_job_board():
    pass

def insert_job_with_questions(job_info, questions):
    job = a.Job(**job_info)

    questions_sa = []

    for prompt in questions['freeresponse']:
        fr_q_sa = select(a.FreeResponseQuestion).where(a.FreeResponseQuestion.question == prompt)
        if fr_q_sa is None:
            fr_q_sa = a.FreeResponseQuestion(question = prompt)

        questions_sa.append(fr_q_sa)

    '''
    For each dropdown question, we want to get a list of the possibl
    '''
    for question_options in questions['dropdown']:
        question = question_options['question']
        options = question_options['options']

        options_sa = []
        for option in options:
            option_sa = select(a.Option).where(and_(a.Option.text == option['text'], a.Option.value == option['value']))
            if option_sa is None:
                option_sa = a.Option(**option)

            