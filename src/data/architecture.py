import os
import datetime
from enum import Enum
from typing import List
from sqlalchemy import Table
from sqlalchemy import UniqueConstraint, ForeignKey
from sqlalchemy import create_engine
from sqlalchemy import func
from sqlalchemy import String, Boolean, Integer, Float, Column, DateTime
from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import relationship

engine = None

class Base(DeclarativeBase):
    pass

class JobBoard(Base):
    __tablename__ = "jobboard"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)

    name: Mapped[str] = mapped_column(String(30))
    
    jobs: Mapped[List["Job"]] = relationship(back_populates="jobboard")

    def __repr__(self):
        return f"<JobBoard(name='{self.name}')>"

jobquestion_table = Table(
    "jobquestion",
    Base.metadata,
    Column("jobid", ForeignKey("job.id"), primary_key=True),
    Column("questionid", ForeignKey("question.id"), primary_key=True)
)

optionsetoption_table = Table(
    "optionsetoption",
    Base.metadata,
    Column("optionid", ForeignKey("option.id"), primary_key=True),
    Column("optionsetid", ForeignKey("optionset.id"), primary_key=True)
)

checkboxanswers_table = Table(
    "checkboxanswers",
    Base.metadata,
    Column("checkboxquestionid", ForeignKey("checkboxquestion.id"), primary_key=True),
    Column("answerasoptionid", ForeignKey("option.id"), primary_key=True)
)

class OptionSet(Base):
    __tablename__ = "optionset"

    id: Mapped[int] = mapped_column(primary_key=True)
    optionshash: Mapped[str] = mapped_column(String, unique=True, nullable=False)

    radiobuttonquestions: Mapped[List["RadioButtonQuestion"]] = relationship(back_populates="optionset")
    dropdownquestions: Mapped[List["DropDownQuestion"]] = relationship(back_populates="optionset")
    checkboxquestions: Mapped[List["CheckBoxQuestion"]] = relationship(back_populates="optionset")

    options: Mapped[List["Option"]] = relationship(
        secondary=optionsetoption_table, back_populates="optionsets"
    )

class DocumentRequirementStatus(Base):
    __tablename__ = "documentrequirementstatus"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    status: Mapped[str] = mapped_column(String, unique=True, nullable=False)

    def __repr__(self):
        return f"<DocumentRequirementStatus(status='{self.status}')>"

DOCUMENT_STATUS_VALUES = [
    "Not requested nor required",
    "Requested but not required",
    "Required"
]

class Job(Base):
    __tablename__ = "job"

    id: Mapped[int] = mapped_column(primary_key=True)

    # columns obtained from parse_sub_title_text
    companyname: Mapped[str] = mapped_column(String)
    location: Mapped[str] = mapped_column(String, nullable=True)
    isarepost: Mapped[bool] = mapped_column(Boolean, nullable=True)
    postedtimeago: Mapped[str] = mapped_column(String, nullable=True)
    numapplicants: Mapped[str] = mapped_column(String, nullable=True)

    # columns obtained from parse_first_line_text
    salarylowerbound: Mapped[int] = mapped_column(Integer, nullable=True)
    salaryupperbound: Mapped[int] = mapped_column(Integer, nullable=True)
    workplacetype: Mapped[str] = mapped_column(String, nullable=True)
    jobtype: Mapped[str] = mapped_column(String, nullable=True)
    explevel: Mapped[str] = mapped_column(String, nullable=True)

    # columns obtained from parse_second_line_text
    numemployees: Mapped[str] = mapped_column(String, nullable=True)
    industry: Mapped[str] = mapped_column(String, nullable=True)

    jobtitle: Mapped[str] = mapped_column(String)
    description: Mapped[str] = mapped_column(String)
    appsubmitted: Mapped[bool] = mapped_column(Boolean)
    extjobid: Mapped[int] = mapped_column(Integer)

    resumerequirementstatusid: Mapped[int] = mapped_column(
        Integer, ForeignKey('documentrequirementstatus.id'), nullable=False
    )
    coverletterrequirementstatusid: Mapped[int] = mapped_column(
        Integer, ForeignKey('documentrequirementstatus.id'), nullable=False
    )

    jobboardid = Column(Integer, ForeignKey('jobboard.id'))

    createdat: Mapped[datetime.datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    preferencescore: Mapped[float] = mapped_column(Float, nullable=True)

    jobboard: Mapped["JobBoard"] = relationship(back_populates="jobs")
    
    questions: Mapped[List["Question"]] = relationship(
        secondary=jobquestion_table, back_populates="jobs"
    )

    resume_status: Mapped["DocumentRequirementStatus"] = relationship(foreign_keys=[resumerequirementstatusid])
    cover_letter_status: Mapped["DocumentRequirementStatus"] = relationship(foreign_keys=[coverletterrequirementstatusid])

    UniqueConstraint("jobboardid", "extjobid")

    def __repr__(self):
        return f"<Job(title='{self.title}', company_name='{self.company_name}')>"

QuestionType = Enum('QuestionType', ['FREERESPONSE', 'RADIOBUTTON', 'DROPDOWN', 'CHECKBOX'])

class Question(Base):
    __tablename__ = "question"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    question: Mapped[str] = mapped_column(String)
    type: Mapped[str]

    jobs: Mapped[List["Job"]] = relationship(
        secondary=jobquestion_table, back_populates="questions"
    )

    __mapper_args__ = {
        "polymorphic_identity": "question",
        "polymorphic_on": "type"
    }
    
    def __repr__(self):
        return f"<Question(question='{self.question}', type='{self.type}')>"

class FreeResponseQuestion(Question):
    __tablename__ = 'freeresponsequestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    ismultiline: Mapped[bool] = mapped_column(Boolean)
    answer: Mapped[str] = mapped_column(String, nullable=True)

    __mapper_args__ = {
        "polymorphic_identity": "free response",
    }

class RadioButtonQuestion(Question):
    __tablename__ = 'radiobuttonquestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    optionsetid: Mapped[int] = Column(Integer, ForeignKey("optionset.id"))
    answerasoptionid = Column(Integer, ForeignKey('option.id'), nullable=True)

    __mapper_args__ = {
        "polymorphic_identity": "radio buttons",
    }

    optionset: Mapped["OptionSet"] = relationship(back_populates="radiobuttonquestions")


class DropDownQuestion(Question):
    __tablename__ = 'dropdownquestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    optionsetid: Mapped[int] = Column(Integer, ForeignKey("optionset.id"))
    answerasoptionid = Column(Integer, ForeignKey('option.id'), nullable=True)

    __mapper_args__ = {
        "polymorphic_identity": "drop down",
    }

    optionset: Mapped["OptionSet"] = relationship(back_populates="dropdownquestions")


class CheckBoxQuestion(Question):
    __tablename__ = 'checkboxquestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    optionsetid: Mapped[int] = Column(Integer, ForeignKey("optionset.id"))

    __mapper_args__ = {
        "polymorphic_identity": "checkbox",
    }

    optionset: Mapped["OptionSet"] = relationship(back_populates="checkboxquestions")

    checkboxanswers: Mapped[List["Option"]] = relationship(
        secondary=checkboxanswers_table, back_populates="checkboxquestions"
    )


class Option(Base):
    __tablename__ = 'option'

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    text: Mapped[str] = mapped_column(String)
    value: Mapped[str] = mapped_column(String)

    optionsets: Mapped[List[OptionSet]] = relationship(
        secondary=optionsetoption_table, back_populates="options"
    )

    checkboxquestions: Mapped[List["CheckBoxQuestion"]] = relationship(
        secondary=checkboxanswers_table, back_populates="checkboxanswers"
    )

def populate_document_statuses():
    from sqlalchemy.orm import Session
    with Session(engine) as session:
        for status_value in DOCUMENT_STATUS_VALUES:
            if not session.query(DocumentRequirementStatus).filter_by(status=status_value).first():
                session.add(DocumentRequirementStatus(status=status_value))
        session.commit()


def initialize_engine(db_path):
    """
    Initializes the engine with a given database path.
    
    Args:
        db_path (str): The path to the SQLite database file.
    """
    global engine  # Ensure the function modifies the module-level 'engine'

    # Ensure the directory exists
    directory = os.path.dirname(db_path)
    if not os.path.exists(directory):
        os.makedirs(directory)  # Create the directory if it doesn't exist

    engine = create_engine(f'sqlite:///{db_path}', echo=True)
    Base.metadata.create_all(engine)  # Create tables if they don't exist yet

    populate_document_statuses()