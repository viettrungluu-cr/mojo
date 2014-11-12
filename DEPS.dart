import gclient_utils
import os

path = gclient_utils.FindGclientRoot(os.getcwd())
execfile(os.path.join(path, 'src', 'DEPS'))  # Include proper Mojo DEPS.

# Now we need to add in Dart.

vars.update({
  'dart_svn': 'https://dart.googlecode.com',
  'dart_revision': '41678',
})

deps.update({
  'src/dart/runtime':
   Var('dart_svn') + '/svn/branches/bleeding_edge/dart/runtime' + '@' + Var('dart_revision'),

  'src/dart/sdk/lib':
   Var('dart_svn') + '/svn/branches/bleeding_edge/dart/sdk/lib' + '@' + Var('dart_revision'),

  'src/dart/tools':
   Var('dart_svn') + '/svn/branches/bleeding_edge/dart/tools' + '@' + Var('dart_revision'),
})

allowed_hosts += ['dart.googlecode.com']
