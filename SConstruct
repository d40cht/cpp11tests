import os, sys
import os.path

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
#Export( 'buildConfig' )

#SConscript("main.scons", variant_dir=buildConfig.buildDir(), duplicate=0)

#import sys
#import os.path

#Import('buildConfig')

class NavStaticLibrary(object):
    def interfacePath( self ):
        return os.path.join( self.path, "interface" )
    
    def __init__( self, build, name, path, libDependencies ):
        self.name = name
        self.path = path
        
        localEnv = build.env.Clone()
        allIncludePaths = ["#" + self.interfacePath()] + ["#" + dep.interfacePath() for dep in libDependencies]
        localEnv.Append(CPPPATH = allIncludePaths)

        self.underlying = localEnv.StaticLibrary( name, Glob( os.path.join( path, "source", "*.cpp" ) ) )
        
        testPath = os.path.join( self.path, "test" )
        if os.path.exists( testPath ):
            testName = name + "_test"
            testApp = NavApplication( build, testName, testPath, [self] + libDependencies )
            testAlias = Alias( testName, [testApp.underlying], testApp.underlying[0].abspath )
            build.allTests.append( (testName, testAlias) )
            AlwaysBuild( testAlias )
            
class NavApplication(object):
    def includePath( self ):
        return os.path.join( self.path, "include" )
        
    def __init__( self, build, name, path, libDependencies ):
        self.name = name
        self.path = path
        
        localEnv = build.env.Clone()
        allIncludePaths = ["#" + self.includePath()] + ["#" + dep.interfacePath() for dep in libDependencies]
        localEnv.Append(CPPPATH = allIncludePaths)

        self.underlying = localEnv.Program( name, Glob( os.path.join( path, "source", "*.cpp" ) ), LIBS=[dep.underlying for dep in libDependencies] )

class NavBuild(object):
    def __init__( self, buildConfig ):
        self.allTests = []
        self.buildConfig = buildConfig
        self.env = buildConfig.env
        
        self.env['CXX'] = "/usr/bin/g++-4.7"
        if buildConfig.mode.value == "debug":
            self.env.Append(CXXFLAGS = '-std=c++11 -g')
        else:
            self.env.Append(CXXFLAGS = '-std=c++11 -O2')

        self.scriptRoot = Dir("#").abspath

    def staticLibrary( self, name, path, libDependencies ):
        return NavStaticLibrary( self, name, path, libDependencies )
        
    def application( self, name, path, libDependencies ):
        return NavApplication( self, name, path, libDependencies )
        
    def finalise( self ):
        allTestAlias = Alias( "test", [t for tn, t in self.allTests] )
        AlwaysBuild( allTestAlias )

nb = NavBuild(buildConfig)

# Declaration of libraries
utility = nb.staticLibrary( 'utility', 'libraries/utility', [] )
functionalCollections = nb.staticLibrary( 'functionalcollections', 'libraries/functionalcollections', [utility] )
dataStructures = nb.staticLibrary( 'datastructures', 'libraries/datastructures', [utility] )
program = nb.application( 'test', 'applications/simple', [utility, functionalCollections, dataStructures] )

nb.finalise()


