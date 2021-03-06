#!/usr/bin/env python

# Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

import unittest

script = __import__('gtest-parallel-wrapper')  # pylint: disable=invalid-name


class GtestParallelWrapperTest(unittest.TestCase):
  @classmethod
  def _Expected(cls, gtest_parallel_args):
    return ['--shard_index=0', '--shard_count=1'] + gtest_parallel_args

  def testOverwrite(self):
    result = script.ParseArgs(['--timeout=123', 'exec', '--timeout', '124'])
    expected = self._Expected(['--timeout=124', 'exec'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testMixing(self):
    result = script.ParseArgs(
        ['--timeout=123', '--param1', 'exec', '--param2', '--timeout', '124'])
    expected = self._Expected(
        ['--timeout=124', 'exec', '--', '--param1', '--param2'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testMixingPositional(self):
    result = script.ParseArgs(['--timeout=123', 'exec', '--foo1', 'bar1',
                               '--timeout', '124', '--foo2', 'bar2'])
    expected = self._Expected(['--timeout=124', 'exec', '--', '--foo1', 'bar1',
                               '--foo2', 'bar2'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testDoubleDash1(self):
    result = script.ParseArgs(
        ['--timeout', '123', 'exec', '--', '--timeout', '124'])
    expected = self._Expected(
        ['--timeout=123', 'exec', '--', '--timeout', '124'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testDoubleDash2(self):
    result = script.ParseArgs(['--timeout=123', '--', 'exec', '--timeout=124'])
    expected = self._Expected(['--timeout=123', 'exec', '--', '--timeout=124'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testArtifacts(self):
    result = script.ParseArgs(['exec', '--store-test-artifacts',
                               '--output_dir', '/tmp/foo'])
    expected = self._Expected(['--output_dir=/tmp/foo', 'exec', '--',
                               '--test_artifacts_dir=/tmp/foo/test_artifacts'])
    self.assertEqual(result.gtest_parallel_args, expected)
    self.assertEqual(result.output_dir, '/tmp/foo')
    self.assertRegexpMatches(result.test_artifacts_dir,
                             '/tmp/foo.test_artifacts')

  def testNoDirsSpecified(self):
    result = script.ParseArgs(['exec'])
    self.assertEqual(result.output_dir, None)
    self.assertEqual(result.test_artifacts_dir, None)

  def testOutputDirSpecified(self):
    result = script.ParseArgs(['exec', '--output_dir', '/tmp/foo'])
    self.assertEqual(result.output_dir, '/tmp/foo')
    self.assertEqual(result.test_artifacts_dir, None)

  def testJsonTestResults(self):
    result = script.ParseArgs(['--isolated-script-test-output', '/tmp/foo',
                               'exec'])
    expected = self._Expected(['--dump_json_test_results=/tmp/foo', 'exec'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testShortArg(self):
    result = script.ParseArgs(['-d', '/tmp/foo', 'exec'])
    expected = self._Expected(['--output_dir=/tmp/foo', 'exec'])
    self.assertEqual(result.gtest_parallel_args, expected)
    self.assertEqual(result.output_dir, '/tmp/foo')

  def testBoolArg(self):
    result = script.ParseArgs(['--gtest_also_run_disabled_tests', 'exec'])
    expected = self._Expected(['--gtest_also_run_disabled_tests', 'exec'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testNoArgs(self):
    result = script.ParseArgs(['exec'])
    expected = self._Expected(['exec'])
    self.assertEqual(result.gtest_parallel_args, expected)

  def testDocExample(self):
    result = script.ParseArgs([
        'some_test', '--some_flag=some_value', '--another_flag',
        '--output_dir=SOME_OUTPUT_DIR', '--store-test-artifacts',
        '--isolated-script-test-output=SOME_DIR',
        '--isolated-script-test-perf-output=SOME_OTHER_DIR',
        '--foo=bar', '--baz'])
    expected = self._Expected([
        '--output_dir=SOME_OUTPUT_DIR', '--dump_json_test_results=SOME_DIR',
        'some_test', '--',
        '--test_artifacts_dir=SOME_OUTPUT_DIR/test_artifacts',
        '--some_flag=some_value', '--another_flag',
        '--isolated-script-test-perf-output=SOME_OTHER_DIR',
        '--foo=bar', '--baz'])
    self.assertEqual(result.gtest_parallel_args, expected)


if __name__ == '__main__':
  unittest.main()
