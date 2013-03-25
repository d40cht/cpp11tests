import os, sys

class OptVar(object):
    def __init__( self, name, alts ):
        self.name = name
        self.alts = alts
        self.value = None
        
    def addToVars( self, opts ):
        opts.Add( self.name, "/".join(self.alts) )
        
    def read(self, env):
        if self.name not in env or env[self.name] not in self.alts:
            print "Missing argument %s (alternatives: %s)" % (self.name, "/".join(self.alts))
            sys.exit(1)
        self.value = env[self.name]

class BuildConfig(object):
    def __init__(self):
        self.mode = OptVar( "mode", ["debug", "release"] )
        #self.arch = OptVar( "arch", ["LinuxPC", "LinuxARM"] )
        
        opts = Variables(None, ARGUMENTS)
        self.mode.addToVars( opts )
        #self.arch.addToVars( opts )
        
        self.env=Environment(variables=opts)
        
        self.mode.read(self.env)
        #self.arch.read(self.env)
        
    def buildDir( self ):
        #return os.path.join( "build", self.arch.value, self.mode.value )
        return os.path.join( "build", self.mode.value )
        
        
buildConfig = BuildConfig()
    

# Make env available to nested SConscript targets
Export( 'buildConfig' )

SConscript("main.scons", variant_dir=buildConfig.buildDir(), duplicate=0)


