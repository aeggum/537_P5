import subprocess, os, glob
from test import Test

class BuildTest(Test):
   name = "build"
   description = "build project using make"
   timeout = 15
   targets = ["libmfs.so", "server"]

   def run(self):
      self.clean_project(self.targets + ["*.o"])
      self.build_project(self.targets)
      self.done()

   def build_project(self, files=[]):
      self.log.write("make\n")
      self.log.flush()
      child = subprocess.Popen("make", cwd=self.project_path, stdout=self.log,
            stderr=self.log)
      status = child.wait()
      if status != 0:
         self.fail("make failed (error " + str(status) + ")")
      missing_files = []
      for f in files:
         if not os.path.exists(self.project_path + "/" + f):
            self.fail("required file '" + f + "' does not exist after make")
            missing_files.append(f)
      if len(missing_files) > 0:
         child = subprocess.Popen(["make"] + missing_files,
               cwd=self.project_path, stdout=self.log, stderr=self.log)
         status = child.wait()


   def clean_project(self, files=[]):
      self.log.write("make clean\n")
      self.log.flush()
      child = subprocess.Popen(["make", "clean"], cwd=self.project_path,
            stdout=self.log, stderr=self.log)
      status = child.wait()
      if status != 0:
         self.fail("make clean failed")
      for pattern in files:
         for f in glob.glob(pattern):
            if os.path.exists(self.project_path + "/" + f):
               self.warn("file '" + f + "' exists after make clean, removing")
               os.remove(self.project_path + "/" + f)

