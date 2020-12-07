project('grman_project_1', 'cpp',
        license : ['Boost'],
        version : '0.1',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

thread_dep = dependency('threads')

# begin asio: https://think-async.com/Asio/
add_global_arguments('-I' + get_option('asio_include_dir'), language : 'cpp')
add_global_arguments('-DASIO_STANDALONE', language : 'cpp')
add_global_arguments('-DASIO_NO_DEPRECATED', language : 'cpp')
# end asio

# CLI11: https://github.com/CLIUtils/CLI11
add_global_arguments('-I' + get_option('cli11_include_dir'), language : 'cpp')

# spdlog: https://github.com/gabime/spdlog
# add_global_arguments('-I' + get_option('spdlog_include_dir'), language : 'cpp')
spdlog = dependency('spdlog')

# Catch2: https://github.com/catchorg/Catch2
# add_global_arguments('-I' + get_option('catch2_include_dir'), language : 'cpp')
catch2 = dependency('catch2')

# fmt: https://github.com/fmtlib/fmt
# add_global_arguments('-I' + get_option('fmt_include_dir'), language : 'cpp')
fmt = dependency('fmt')

inc_dir = include_directories('include')

src = ['src/message_buffer.cpp', 'src/messages.cpp']

executable('ring_voting',
           sources : ['src/main.cpp'] + src,
           include_directories : inc_dir,
           dependencies : [thread_dep, fmt, spdlog]
          )

executable('unit_tests',
           sources : ['src/test_main.cpp'] + src,
           include_directories : inc_dir,
           dependencies : [thread_dep, fmt, spdlog, catch2],
           cpp_args: '-DUNIT_TEST'
          )
