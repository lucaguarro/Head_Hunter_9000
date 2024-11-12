from typing import List, Optional, Tuple
import data.architecture as da
from sqlalchemy.orm import sessionmaker
from sqlalchemy.orm import aliased
from sqlalchemy import func, tuple_, and_
from sqlalchemy.exc import NoResultFound

session = None

def initialize_session():
    global session
    Session = sessionmaker(bind=da.engine)
    session = Session()

def create_or_get_job_board(jobboardname):
    jobboard = session.query(da.JobBoard).filter_by(name=jobboardname).first()
    if not jobboard:
        jobboard = da.JobBoard(name=jobboardname)
    
    return jobboard

def create_job(job_info, jobboard_sa, questions_sa):
    job = da.Job(**job_info)
    job.jobboard = jobboard_sa
    job.questions = questions_sa
    session.add(job)

def commit():
    session.commit()

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

def create_options_hash(options_sa):
    option_ids = sorted([o.id for o in options_sa])
    result = '-'.join(map(str, option_ids))
    return result

def does_optionset_exist(options_hash):
    existing_optionset = session.query(da.OptionSet).filter(
        da.OptionSet.optionshash == options_hash
    )
    return existing_optionset.first()

def make_optionset(options_sa, options_hash):
    new_optionset = da.OptionSet(optionshash = options_hash)
    session.add(new_optionset)
    new_optionset.options = options_sa
    # session.commit()

    return new_optionset

def does_question_exist(question_text, question_type, optionset = None, ismultiline = None):

    if question_type == da.QuestionType.FREERESPONSE:
        existing_question = session.query(da.FreeResponseQuestion).filter(
            da.FreeResponseQuestion.question == question_text,
            da.FreeResponseQuestion.ismultiline == ismultiline
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
    elif question_type == da.QuestionType.CHECKBOX:
        existing_question = session.query(da.CheckBoxQuestion).filter(
            da.DropDownQuestion.question == question_text,
            da.DropDownQuestion.optionset == optionset
        )

    return existing_question.first()

def create_question_and_options(prompt_and_options, question_type):
    prompt = prompt_and_options[0]
    options = prompt_and_options[1]

    options_sa, was_option_created = insert_options(options)
    if was_option_created:
        flush()

    options_hash = create_options_hash(options_sa)
    was_optionset_created = False
    if was_option_created:
        optionset_sa = make_optionset(options_sa, options_hash)
        was_optionset_created = True
    else:
        optionset_sa = does_optionset_exist(options_hash)
        if not optionset_sa:
            optionset_sa = make_optionset(options_sa, options_hash)
            was_optionset_created = True
    
    if was_optionset_created:
        flush()

    if was_optionset_created:
        question_sa = create_question(prompt, question_type, optionset_sa)
    else:
        question_sa = does_question_exist(prompt, question_type, optionset_sa)
    if not question_sa:
        question_sa = create_question(prompt, question_type, optionset_sa)
    
    return question_sa


def create_question(question_text, question_type, optionset = None, ismultiline = None):
    if question_type == da.QuestionType.FREERESPONSE:
        new_question = da.FreeResponseQuestion(
            question = question_text,
            ismultiline = ismultiline
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
    elif question_type == da.QuestionType.CHECKBOX:
        new_question = da.CheckBoxQuestion(
            question = question_text,
            optionset = optionset
        )

    return new_question

def get_document_requirement_status_id(status_name):
    """
    Retrieve the id of the document requirement status given the status name.
    """
    status = session.query(da.DocumentRequirementStatus).filter_by(status=status_name).first()
    return status.id if status else None


def get_dropdown_answer(question_prompt: str, options: List[dict]) -> Tuple[bool, Optional[str]]:
    """
    Determines if a dropdown question exists in the database based on the question prompt and options.
    Each option is a dictionary with 'text' and 'value' keys.

    Args:
        question_prompt (str): The exact text of the dropdown question.
        options (List[dict]): A list of dictionaries, each containing:
            - "text": The display text of the option.
            - "value": The value attribute of the option.

    Returns:
        Tuple[bool, Optional[str]]: 
            - The first element is a boolean indicating whether the dropdown question exists.
            - The second element is the `answerasoptionid` if the question exists; otherwise, `None`.
    """
    # Step 1: Retrieve Option IDs based on provided option texts and values
    # Construct a list of tuples (text, value) from the options
    option_tuples = [(opt['text'], opt['value']) for opt in options]

    # Query the Option table for matching (text, value) pairs
    option_ids_query = session.query(da.Option.id).filter(
        tuple_(da.Option.text, da.Option.value).in_(option_tuples)
    )
    option_ids = [option_id for (option_id,) in option_ids_query.all()]

    # Check if all provided (text, value) pairs were found
    if len(option_ids) != len(option_tuples):
        # Identify which options are missing
        retrieved_options = session.query(da.Option.text, da.Option.value).filter(
            tuple_(da.Option.text, da.Option.value).in_(option_tuples)
        ).all()
        retrieved_option_tuples = set(retrieved_options)
        provided_option_tuples = set(option_tuples)
        missing_options = provided_option_tuples - retrieved_option_tuples
        print(f"Missing options in the database: {missing_options}")
        return False, None

    # Step 2: Identify OptionSet IDs that exactly match the provided Option IDs
    # Find OptionSets that contain exactly the provided Option IDs
    matching_optionset_ids = (
        session.query(da.optionsetoption_table.c.optionsetid)
        .filter(da.optionsetoption_table.c.optionid.in_(option_ids))
        .group_by(da.optionsetoption_table.c.optionsetid)
        .having(func.count(da.optionsetoption_table.c.optionid) == len(option_ids))
        .all()
    )

    # Extract OptionSet IDs from the query result
    matching_optionset_ids = [optionset_id for (optionset_id,) in matching_optionset_ids]

    if len(matching_optionset_ids) != 1:
        # Either no matching OptionSet found or multiple found, which shouldn't happen
        if len(matching_optionset_ids) == 0:
            print("No OptionSet matches the exact set of provided options.")
        else:
            print("Multiple OptionSets match the exact set of provided options.")
        return False, None

    # Extract the single matching OptionSet ID
    matching_optionset_id = matching_optionset_ids[0]

    # Step 3: Verify the existence of the DropDownQuestion with matching question_prompt and optionset_id
    SelectedOption = aliased(da.Option)

    result = (
        session.query(SelectedOption.value)
        .select_from(da.DropDownQuestion)  # Set DropDownQuestion as the base
        .outerjoin(SelectedOption, da.DropDownQuestion.answerasoptionid == SelectedOption.id)
        .filter(
            and_(
                da.DropDownQuestion.question == question_prompt,  # Access via DropDownQuestion
                da.DropDownQuestion.optionsetid == matching_optionset_id
            )
        )
        .one_or_none()
    )

    if result is None:
        # No DropDownQuestion found matching the criteria
        return False, None
    else:
        # A DropDownQuestion exists; check if the selected_option_value is NULL
        answer = result.value  # This will be None if answerasoptionid is NULL
        return True, answer

def get_radiobutton_answer(question_prompt: str, options: List[dict]) -> Tuple[bool, Optional[str]]:
    # Step 1: Retrieve Option IDs based on provided option texts and values
    # Construct a list of tuples (text, value) from the options
    option_tuples = [(opt['text'], opt['value']) for opt in options]

    # Query the Option table for matching (text, value) pairs
    option_ids_query = session.query(da.Option.id).filter(
        tuple_(da.Option.text, da.Option.value).in_(option_tuples)
    )
    option_ids = [option_id for (option_id,) in option_ids_query.all()]

    # Check if all provided (text, value) pairs were found
    if len(option_ids) != len(option_tuples):
        # Identify which options are missing
        retrieved_options = session.query(da.Option.text, da.Option.value).filter(
            tuple_(da.Option.text, da.Option.value).in_(option_tuples)
        ).all()
        retrieved_option_tuples = set(retrieved_options)
        provided_option_tuples = set(option_tuples)
        missing_options = provided_option_tuples - retrieved_option_tuples
        print(f"Missing options in the database: {missing_options}")
        return False, None

    # Step 2: Identify OptionSet IDs that exactly match the provided Option IDs
    # Find OptionSets that contain exactly the provided Option IDs
    matching_optionset_ids = (
        session.query(da.optionsetoption_table.c.optionsetid)
        .filter(da.optionsetoption_table.c.optionid.in_(option_ids))
        .group_by(da.optionsetoption_table.c.optionsetid)
        .having(func.count(da.optionsetoption_table.c.optionid) == len(option_ids))
        .all()
    )

    # Extract OptionSet IDs from the query result
    matching_optionset_ids = [optionset_id for (optionset_id,) in matching_optionset_ids]

    if len(matching_optionset_ids) != 1:
        # Either no matching OptionSet found or multiple found, which shouldn't happen
        if len(matching_optionset_ids) == 0:
            print("No OptionSet matches the exact set of provided options.")
        else:
            print("Multiple OptionSets match the exact set of provided options.")
        return False, None

    # Extract the single matching OptionSet ID
    matching_optionset_id = matching_optionset_ids[0]

    # Step 3: Verify the existence of the RadioButtonQuestion with matching question_prompt and optionset_id
    SelectedOption = aliased(da.Option)

    result = (
        session.query(SelectedOption.value)
        .select_from(da.RadioButtonQuestion)  # Set RadioButtonQuestion as the base
        .outerjoin(SelectedOption, da.RadioButtonQuestion.answerasoptionid == SelectedOption.id)
        .filter(
            and_(
                da.RadioButtonQuestion.question == question_prompt,  # Access via RadioButtonQuestion
                da.RadioButtonQuestion.optionsetid == matching_optionset_id
            )
        )
        .one_or_none()
    )

    if result is None:
        # No DropDownQuestion found matching the criteria
        return False, None
    else:
        # A DropDownQuestion exists; check if the selected_option_value is NULL
        answer = result.value  # This will be None if answerasoptionid is NULL
        return True, answer

def get_free_response_answer(question_text):
    try:
        question = session.query(da.FreeResponseQuestion).filter(
            da.FreeResponseQuestion.question == question_text
        ).one()
        return True, question.answer
    except NoResultFound:
        return False, None
    
def flush():
    session.flush()

# def does_optionset_exist(options_sa):
#     option_ids = [o.id for o in options_sa]

#     subquery = session.query(da.OptionSet).join(
#         da.optionsetoption_table,
#         da.Option.id == da.optionsetoption_table.c.optionid
#     ).filter(
#         da.Option.id.in_(option_ids)
#     ).group_by(da.OptionSet).having(
#         func.count(da.optionsetoption_table.c.optionid) == len(option_ids)
#     ).subquery()

#     matching_optionset = session.query(da.OptionSet).filter(da.OptionSet.id.in_(subquery)).first()
#     return matching_optionset