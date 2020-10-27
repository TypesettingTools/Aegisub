test_sources = files(
  'ctest.c',
  'tstdtmod.cpp',
  'testdata.cpp',
  'datamap.cpp',
  'uperf.cpp',
  'ucln_ct.c',
)

test_args = '-DT_CTEST_IMPLEMENTATION'

test_lib = library('icutest',
  test_sources,
  include_directories: [incdir, toolinc],
  c_args: test_args,
  cpp_args: test_args,
  link_with: [data_lib, tu_lib, uc_lib],
)
