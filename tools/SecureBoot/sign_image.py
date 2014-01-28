#!/usr/bin/env python

# Copyright (c) 2014, Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


"""Signs a Barebox image using SHA1 hashing and RSA.

The provided image is first hashed using SHA1. This hash is then
signed using the provided RSA private key. The signing is performed
by a system call to:

  openssl dgst -sha1 -sign private.key image

Finally, the RSA signature and some other information (image length, etc)
is prepended onto the image and output as a new file.
"""


import os
import struct
import subprocess
import sys
import threading


# The format string used to pack integers. This defines a 32-bit little
# endian unsigned value.
struct_int_format = '<L'


# A magic number at the start of the header, to allow early exit.
magic_number = struct.pack(struct_int_format, 0xDA4EAD32)


class TimedCommandRunner(object):
  """Runs system commands with a timeout."""

  def Run(self, command, timeout):
    """Run a given command as a subprocess, terminating it if a timeout expires.

    Args:
      command: The system command to urn.
      timeout: The number of seconds to allow the process before terminating.

    Returns:
      The command's output (stdout) if the process finishes, else None.
    """

    self.output = None
    self.return_code = None

    def RunProcess(command):
      self.proc = subprocess.Popen(command, stdout=subprocess.PIPE)
      (stdout, _) = self.proc.communicate()
      self.output = stdout
      self.return_code = self.proc.returncode

    thread = threading.Thread(target=RunProcess, args=(command,))
    thread.start()

    thread.join(timeout)
    if thread.is_alive():
      print 'Error: Timed out waiting for openssl. Killing process.'
      self.proc.terminate()
      thread.join()
      return None

    if self.return_code != 0:
      print('Error: Openssl had non-zero return code: {0}.'.format(
          self.return_code))
      return None

    return self.output


def _GetSignature(private_key_path, image_path):
  """Returns the SHA1 RSA signature for the given image."""

  openssl_command = ['openssl', 'dgst', '-sha1', '-sign', private_key_path,
                     image_path]
  runner = TimedCommandRunner()
  return runner.Run(openssl_command, 10)


def main():
  if len(sys.argv) < 3:
    print 'Usage {0} private_key.pem image.bin'.format(sys.argv[0])
    return -1

  private_key_path = sys.argv[1]
  image_path = sys.argv[2]

  if not os.path.isfile(private_key_path):
    print 'Error: Cannot find file {0}'.format(private_key_path)
    return -1

  if not os.path.isfile(image_path):
    print 'Error: Cannot find file {0}'.format(image_path)
    return -1

  signature = _GetSignature(private_key_path, image_path)
  if signature is None:
    return 1

  stat_info = os.stat(image_path)
  image_length = struct.pack(struct_int_format, stat_info.st_size)

  output_image_path = image_path + '.signed'

  print 'Generating signed image to {0}'.format(output_image_path)

  with open(output_image_path, 'wb') as f:
    f.write(magic_number)
    f.write(image_length)
    f.write(signature)
    with open(image_path, 'rb') as image_file:
      f.write(image_file.read())

  return 0


if __name__ == '__main__':
  main()
