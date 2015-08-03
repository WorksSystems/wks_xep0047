import os

env = Environment()
srcs=['source/xmpp_ibb.c', 'source/xmppclient.c']
env.AppendUnique(CPPPATH=['/home/user/github/libstrophe', '/home/user/github/libstrophe/src', 'include', '.'])
env.AppendUnique(LIBS=['strophe'], LIBPATH=['/home/user/github/libstrophe/.libs'], RPATH=['/home/user/github/libstrophe/.libs'])
env.SharedLibrary('wksxmpp', srcs)
env.StaticLibrary('wksxmpp', srcs)
menv = Environment()
menv.AppendUnique(CPPPATH=['/home/user/github/libstrophe', '/home/user/github/libstrophe/src', 'include', '.'])
menv.AppendUnique(LIBS=['wksxmpp', 'strophe'], LIBPATH=['.', '/home/user/github/libstrophe/.libs'], RPATH=['.', '/home/user/github/libstrophe/.libs'])
menv.Program('example/main.c')
