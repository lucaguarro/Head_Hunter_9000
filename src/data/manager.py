import data.architecture as da
from sqlalchemy.orm import sessionmaker
from sqlalchemy import func

Session = sessionmaker(bind=da.engine)
session = Session()

def insert_options(options):
    
    options_sa = []
    was_option_created = False

    for option in options:

        # Try to retrieve an existing option with the same text and value
        existing_option = session.query(da.Option).filter_by(**option).first()

        if existing_option is None:
            print("no option found")
            # Option doesn't exist, create and insert it
            new_option = da.Option(**option)
            session.add(new_option)
            # session.commit()
            was_option_created = True
            
            # Now you have a reference to the newly inserted option
            existing_option = new_option
        else:
            print("option found")

        options_sa.append(existing_option)

    return options_sa, was_option_created

def does_optionset_exist(options_sa):
    option_ids = [o.id for o in options_sa]

    subquery = session.query(da.OptionSet).join(
        da.optionsetoption_table,
        da.Option.id == da.optionsetoption_table.c.optionid
    ).filter(
        da.Option.id.in_(option_ids)
    ).group_by(da.OptionSet).having(
        func.count(da.optionsetoption_table.c.optionid) == len(option_ids)
    ).subquery()

    matching_optionset = session.query(da.OptionSet).filter(da.OptionSet.id.in_(subquery)).first()
    return matching_optionset

def make_optionset(options_sa):
    new_optionset = da.OptionSet()
    session.add(new_optionset)
    new_optionset.options = options_sa
    # session.commit()

    return new_optionset

def does_question_exist(question_text, question_type, optionset):

    if question_type == da.QuestionType.FREERESPONSE:
        existing_question = session.query(da.FreeResponseQuestion).filter(
            da.FreeResponseQuestion.question == question_text,
        )
    elif question_type == da.QuestionType.RADIOBUTTON:
        existing_question = session.query(da.RadioButtonQuestion).filter(
            da.RadioButtonQuestion.question == question_text,
            da.RadioButtonQuestion.optionset == optionset
        )
    elif question_type == da.QuestionType.DROPDOWN:
        existing_question = session.query(da.DropDownQuestion).filter(
            da.DropDownQuestion.question == question_text,
            da.DropDownQuestion.optionset == optionset
        )

    return existing_question.first()

def create_question(question_text, question_type, optionset):
    if question_type == da.QuestionType.FREERESPONSE:
        new_question = da.FreeResponseQuestion(
            question = question_text
        )
    elif question_type == da.QuestionType.RADIOBUTTON:
        new_question = da.RadioButtonQuestion(
            question = question_text,
            optionset = optionset
        )
    elif question_type == da.QuestionType.DROPDOWN:
        new_question = da.DropDownQuestion(
            question = question_text,
            optionset = optionset
        )

    return new_question

def create_job(job_dict):
    job = da.Job(
        **job_dict
    )

    return job