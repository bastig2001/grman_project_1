project('grman_project_1', 'cpp',
        license : ['Boost'],
        version : '1.1',
        default_options : ['cpp_std=c++17', 'warning_level=3', 'werror=true'])

thread_dep = dependency('threads')

# begin asio: https://think-async.com/Asio/
add_global_arguments('-I' + get_option('asio_include_dir'), language : 'cpp')
add_global_arguments('-DASIO_STANDALONE', language : 'cpp')
add_global_arguments('-DASIO_NO_DEPRECATED', language : 'cpp')
# end asio

# Catch2: https://github.com/catchorg/Catch2
# add_global_arguments('-I' + get_option('catch2_include_dir'), language : 'cpp')
# catch2 = dependency('catch2')

# CLI11: https://github.com/CLIUtils/CLI11
add_global_arguments('-I' + get_option('cli11_include_dir'), language : 'cpp')

# cpp-peglib: https://github.com/yhirose/cpp-peglib
add_global_arguments('-I' + get_option('peglib_include_dir'), language : 'cpp')

# fmt: https://github.com/fmtlib/fmt
add_global_arguments(['-I' + get_option('fmt_include_dir'), '-DFMT_HEADER_ONLY'], language : 'cpp')
# fmt = dependency('fmt')

# spdlog: https://github.com/gabime/spdlog
# add_global_arguments('-I' + get_option('spdlog_include_dir'), language : 'cpp')
spdlog = dependency('spdlog')

# toml++: https://github.com/marzer/tomlplusplus/
add_global_arguments('-I' + get_option('toml_include_dir'), language : 'cpp')

inc_dir = include_directories('include')

src = [
    'src/message_buffer.cpp',
    'src/worker.cpp', 
    'src/ring.cpp',
    'src/config.cpp',
    'src/event.cpp',
    'src/concrete_presenter/command_line.cpp',
    'src/concrete_presenter/executor.cpp',
    'src/concrete_presenter/presenter.cpp'
]

unit_tests_src = [
    'src/unit_tests/main.cpp',
    'src/unit_tests/worker.cpp',
    'src/unit_tests/message_buffer.cpp'
]

executable('ring_voting',
           sources : ['src/main.cpp'] + src,
           include_directories : inc_dir,
           dependencies : [thread_dep]
          )

executable('unit_tests',
           sources : unit_tests_src + src,
           include_directories : inc_dir,
           dependencies : [thread_dep],
           cpp_args: ['-DUNIT_TEST', 
                      '-I' + get_option('catch2_include_dir')]
          )
