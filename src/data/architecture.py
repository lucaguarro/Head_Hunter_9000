import datetime
from sqlalchemy import UniqueConstraint, ForeignKey
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

    name: Mapped[str] = mapped_column(String(30), primary_key=True)
    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    
    jobs = relationship("Job", back_populates="job_board")

    def __repr__(self):
        return f"<JobBoard(name='{self.name}')>"

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
    created_at: Mapped[datetime.datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    jobboard_id = Column(Integer, ForeignKey('jobboard.id'))
    jobboard = relationship("JobBoard", back_populates="jobs")

    UniqueConstraint("jobboardid", "extjobid")

    def __repr__(self):
        return f"<Job(title='{self.title}', company_name='{self.company_name}')>"


class Question(Base):
    __tablename__ = "question"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    question: Mapped[str] = mapped_column(String)
    type: Mapped[str]

    jobs = relationship("Job", secondary="jobquestion")

    __mapper_args__ = {
        "polymorphic_identity": "question",
        "polymorphic_on": "type"
    }
    
    def __repr__(self):
        return f"<Question(question_text='{self.question_text}', type='{self.type}')>"


class FreeResponseQuestion(Base):


class JobQuestion(Base):
    __tablename__ = 'jobquestion'

    jobid = Column(Integer, ForeignKey('Job.id'), nullable=False)
    questionid = Column(Integer, ForeignKey('Question.id'), nullable=False)

    job = relationship("job")
    question = relationship("question")