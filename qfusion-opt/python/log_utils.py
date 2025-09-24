import os


class FileLogger:
    def __init__(self, filename):
        self.filename = filename
        self.file = open(filename, "a")

    def write(self, message):
        self.file.write(message)
        self.file.flush()
        os.fsync(self.file)

    def close(self):
        self.file.close()
