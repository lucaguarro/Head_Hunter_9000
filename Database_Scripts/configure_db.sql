INSERT INTO JobBoards (JobBoardName, JobBoardID)
VALUES ("linkedin", 1);

CREATE TRIGGER NoJobBoardUpdates
BEFORE UPDATE ON JobBoards
BEGIN
    SELECT RAISE(FAIL, "updates not allowed");
END;

CREATE TRIGGER NoJobBoardInserts
BEFORE INSERT ON JobBoards
BEGIN
    SELECT RAISE(FAIL, "inserts not allowed");
END;

CREATE TRIGGER NoJobBoardDeletes
BEFORE DELETE ON JobBoards
BEGIN
    SELECT RAISE(FAIL, "deletes not allowed");
END;