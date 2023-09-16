import datetime
from typing import List
from sqlalchemy import Table
from sqlalchemy import UniqueConstraint, ForeignKey
from sqlalchemy import create_engine
from sqlalchemy import func
from sqlalchemy import String, Boolean, Integer, Column, DateTime
from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import relationship

class Base(DeclarativeBase):
    pass

class JobBoard(Base):
    __tablename__ = "jobboard"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)

    name: Mapped[str] = mapped_column(String(30))
    
    jobs = relationship("job", back_populates="jobboard")

    def __repr__(self):
        return f"<JobBoard(name='{self.name}')>"

jobquestion_table = Table(
    "jobquestion",
    Base.metadata,
    Column("jobid", ForeignKey("job.id"), primary_key=True),
    Column("questionid", ForeignKey("question.id"), primary_key=True)
)

optionset_table = Table(
    "optionset",
    Base.metadata,
    Column("optionid", ForeignKey("option.id"), primary_key=True),
    Column("questionid", ForeignKey("question.id"), primary_key=True)
)

class Job(Base):
    __tablename__ = "job"

    id: Mapped[int] = mapped_column(primary_key=True)
    jobtitle: Mapped[str] = mapped_column(String)
    companyname: Mapped[str] = mapped_column(String)
    salary: Mapped[str] = mapped_column(String)
    numemployees: Mapped[str] = mapped_column(String)
    location: Mapped[str] = mapped_column(String)
    workplacetype: Mapped[str] = mapped_column(String)
    description: Mapped[str] = mapped_column(String)
    appsubmitted: Mapped[bool] = mapped_column(Boolean)
    extjobid: Mapped[int] = mapped_column(Integer)
    createdat: Mapped[datetime.datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    jobboardid = Column(Integer, ForeignKey('jobboard.id'))
    jobboard = relationship("JobBoard", back_populates="jobs")
    
    questions: Mapped[List["Question"]] = relationship(
        secondary=jobquestion_table, back_populates="jobs"
    )

    UniqueConstraint("jobboardid", "extjobid")

    def __repr__(self):
        return f"<Job(title='{self.title}', company_name='{self.company_name}')>"


class Question(Base):
    __tablename__ = "question"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    question: Mapped[str] = mapped_column(String)
    type: Mapped[str]

    jobs: Mapped[List["Job"]] = relationship(
        secondary=jobquestion_table, back_populates="questions"
    )
    options: Mapped[List["Option"]] = relationship(
        secondary=optionset_table, back_populates="questions"
    )

    __mapper_args__ = {
        "polymorphic_identity": "question",
        "polymorphic_on": "type"
    }
    
    def __repr__(self):
        return f"<Question(question_text='{self.question_text}', type='{self.type}')>"

class FreeResponseQuestion(Question):
    __tablename__ = 'freeresponsequestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    answer: Mapped[str] = mapped_column(String)

    __mapper_args__ = {
        "polymorphic_identity": "free response",
    }

class RadioButtonsQuestion(Question):
    __tablename__ = 'radiobuttonquestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    answerasoptionid = Column(Integer, ForeignKey('option.id'), nullable=False)

    __mapper_args__ = {
        "polymorphic_identity": "radio buttons",
    }

class DropDownQuestion(Question):
    __tablename__ = 'dropdownquestion'

    id: Mapped[int] = mapped_column(ForeignKey("question.id"), primary_key=True)
    answerasoptionid = Column(Integer, ForeignKey('option.id'), nullable=False)

    __mapper_args__ = {
        "polymorphic_identity": "drop down",
    }

class Option(Base):
    __tablename__ = 'option'

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    text: Mapped[str] = mapped_column(String)
    value: Mapped[str] = mapped_column(String)

    questions: Mapped[List[Question]] = relationship(
        secondary=optionset_table, back_populates="options"
    )

engine = create_engine('sqlite:///example.db', echo=True)
Base.metadata.create_all(engine)