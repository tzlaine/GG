# -*- Python -*-

gigi_version = '0.6.0'

ft_pkgconfig_version = '9.0.0'
ft_version = '2.1.2'
ft_win32_lib_name = 'freetype214MT'

devil_version_string = '1.6.1'
devil_version = ''.join(devil_version_string.split('.'))

sdl_version = '1.2.7'

ogre_version = '1.4.5'

ois_version = '1.0.0'

boost_version_string = '1.34'
def BoostStringToNumber(version_string):
    pieces = version_string.split('.')
    return str(int(pieces[0]) * 100000 + int(pieces[1]) * 100 + (3 <= len(pieces) and int(pieces[2]) or 0))
boost_version = BoostStringToNumber(boost_version_string)
