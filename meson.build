project('type_name', 'cpp', default_options : 'cpp_std=c++17')

src = ['main.cpp']

subprojects = ['subarray']

foreach s : subprojects
    dep = s + '_dep'
    set_variable(dep, subproject(s).get_variable(dep))
endforeach

executable('type_name', src, dependencies : [subarray_dep])

test('test type_name_pt',
  executable('type_name_pt', 'test/type_name_pt.cpp'
          , dependencies : [ subarray_dep]
  )
)
