import subprocess, os

class Failure(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return str(self.value)

class Test:

   IN_PROGRESS = 1
   PASSED = 2
   FAILED = 3

   name = None
   description = None
   timeout = None

   def __init__(self, project_path = None, test_path = None, log = None,
         use_gdb = False, use_valgrind = False):
      self.project_path = project_path
      self.log = log
      self.state = Test.IN_PROGRESS
      self.notices = list()
      self.use_gdb = use_gdb
      self.use_valgrind = use_valgrind
      self.test_path = test_path

   def is_failed(self):
      return self.state == Test.FAILED

   def is_passed(self):
      return self.state == Test.PASSED

   def fail(self, reason = None):
      self.state = Test.FAILED
      if reason is not None:
         self.notices.append(reason)
      self.log.write("test " + self.name + " FAILED" + "\n")
      self.log.flush()

   def warn(self, reason):
      self.notices.append(reason)

   def done(self):
      self.log.flush()
      if not self.is_failed():
         self.state = Test.PASSED

   def __str__(self):
      s = "test " + self.name + " "
      if self.is_failed():
         s += "FAILED"
      elif self.is_passed():
         s += "PASSED"
      s += "\n"
      s += " (" + self.description + ")\n"
      for note in self.notices:
         s += " " + note + "\n"
      return s

   def startexe(self, name, args, libs = None, path = None):
      if path is None:
         path = self.project_path + "/" + name
      if libs is not None:
         os.environ["LD_PRELOAD"] = libs
      args.insert(0, path)
      if self.use_gdb:
         return subprocess.Popen(["xterm",
            "-title", "\"" + " ".join(args) + "\"",
            "-e", "gdb", "--args"] + args,
               cwd=self.project_path)
      if self.use_valgrind:
         return subprocess.Popen(["valgrind"] + args,
               cwd=self.project_path)
      else:
         return subprocess.Popen(args, cwd=self.project_path,
               stdout=self.log, stderr=self.log)
      if libs is not None:
         os.environ["LD_PRELOAD"] = ""
         del os.environ["LD_PRELOAD"]


   def after(self):
      pass

