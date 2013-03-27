import sbt._
import Keys._
import complete.{Parser, RichParser}
import complete.DefaultParsers._

package native
{
    import sbt.std.{TaskStreams}
    
    case class BuildConfig(
        // Debug or release
        val mode : String,
        // Compiler (e.g. g++, clang etc)
        val compiler : String,
        // Target platform (e.g. LinuxPC, RaspberryPI etc)
        val platform : String )
        

    class FunctionWithResultPath( val resultPath : File, val fn : () => File )
    {
        def apply() = fn()
        def runIfNotCached( stateCacheDir : File, inputDeps : Set[File] ) =
        {
            val lazyBuild = FileFunction.cached( stateCacheDir / resultPath.toString , FilesInfo.lastModified, FilesInfo.exists ) 
            { _ =>
                Set( fn() )
            }
            lazyBuild(inputDeps)
            
            resultPath
        }
    }
    
    object FunctionWithResultPath
    {
        def apply( resultPath : File )( fn : File => Unit ) =
        {
            new FunctionWithResultPath( resultPath, () => { fn(resultPath); resultPath } )
        }
    }
    
    trait Compiler
    {
        def findHeaderDependencies( s : TaskStreams[_], buildDirectory : File,  includePaths : Seq[File], sourceFile : File ) : FunctionWithResultPath
        def compileToObjectFile( s : TaskStreams[_], buildDirectory : File, includePaths : Seq[File], sourceFile : File ) : FunctionWithResultPath
        def buildStaticLibrary( s : TaskStreams[_], buildDirectory : File, libName : String, objectFiles : Set[File] ) : FunctionWithResultPath
        def buildExecutable( s : TaskStreams[_], buildDirectory : File, exeName : String, linkPaths : Seq[File], linkLibraries : Seq[String], inputFiles : Set[File] ) : FunctionWithResultPath
    }
    
    class GccCompiler( val compilerPath : File, val archiverPath : File, val linkerPath : File ) extends Compiler
    {
        def findHeaderDependencies( s : TaskStreams[_], buildDirectory : File, includePaths : Seq[File], sourceFile : File ) = FunctionWithResultPath( buildDirectory / (sourceFile.base + ".d") )
        { depFile =>
        
            val includePathArg = includePaths.map( ip => "-I " + ip ).mkString(" ")
            val depCmd = "%s -std=c++11 -M %s %s".format( compilerPath, includePathArg, sourceFile )
            s.log.info( "Executing: " + depCmd )
            val depResult = stringToProcess( depCmd ).lines
            
            // Strip off any trailing backslash characters from the output
            val depFileLines = depResult.map( _.replace( "\\", "" ) )
        
            // Drop the first column and split on spaces to get all the files (potentially several per line )
            val allFiles = depFileLines.flatMap( _.split(" ").drop(1) ).map( x => new File(x.trim) )
            
            IO.write( depFile, allFiles.mkString("\n") )
        }
        
        def compileToObjectFile( s : TaskStreams[_], buildDirectory : File, includePaths : Seq[File], sourceFile : File ) = FunctionWithResultPath( buildDirectory / (sourceFile.base + ".o") )
        { outputFile =>
        
            val includePathArg = includePaths.map( ip => "-I " + ip ).mkString(" ")
            val buildCmd = "%s -std=c++11 -Werror -Wall %s -g -c -o %s %s".format( compilerPath, includePathArg, outputFile, sourceFile )
                           
            s.log.info( "Executing: " + buildCmd )
            buildCmd !!
        }
        
        def buildStaticLibrary( s : TaskStreams[_], buildDirectory : File, libName : String, objectFiles : Set[File] ) =
            FunctionWithResultPath( buildDirectory / ("lib" + libName + ".a") )
            { outputFile =>
            
                val arCmd = "%s -c -r %s %s".format( archiverPath, outputFile, objectFiles.mkString(" ") )
                s.log.info( "Executing: " + arCmd )
                arCmd !!
            }
            
        def buildExecutable( s : TaskStreams[_], buildDirectory : File, exeName : String, linkPaths : Seq[File], linkLibraries : Seq[String], inputFiles : Set[File] ) =
            FunctionWithResultPath( buildDirectory / exeName )
            { outputFile =>
            
                val linkPathArg = linkPaths.map( lp => "-L " + lp ).mkString(" ")
                val libArgs = linkLibraries.map( ll => "-l" + ll ).mkString(" ")
                val linkCmd = "%s -o %s %s %s %s".format( linkerPath, outputFile, inputFiles.mkString(" "), linkPathArg, libArgs )
                s.log.info( "Executing: " + linkCmd )
                linkCmd !!
            }
    }
}

object TestBuild extends Build
{   
    //val compilerExe = SettingKey[File]("compiler", "Path to compiler executable")
    //val archiveExe = SettingKey[File]("archive", "Path to archive executable")
    
    val buildConfig = SettingKey[Option[native.BuildConfig]]("build-config")
    val rootBuildDirectory = TaskKey[File]("root-build-dir", "Build root directory (for the config, not the project)")
    val projectBuildDirectory = TaskKey[File]("project-build-dir", "Build directory for this config and project")
    val stateCacheDirectory = TaskKey[File]("state-cache-dir", "Build state cache directory")
    val projectDirectory = SettingKey[File]("project-dir", "Project directory")
    val sourceDirectory = TaskKey[File]("source-dir", "Source directory")
    val includeDirectories = TaskKey[Seq[File]]("include-dirs", "Include directories")
    val linkDirectories = TaskKey[Seq[File]]("link-dirs", "Link directories")
    val nativeLibraries = TaskKey[Seq[String]]("native-libraries", "All native library dependencies for this project")
    val sourceFiles = TaskKey[Set[File]]("source-files", "All source files for this project")
    val sourceFilesWithDeps = TaskKey[Map[File, Set[File]]]("source-files-with-deps", "All source files for this project")
    val objectFiles = TaskKey[Set[File]]("object-files", "All object files for this project" )
    val nativeCompile = TaskKey[File]("native-compile", "Perform a native compilation for this project" )
    val nativeRun = TaskKey[Unit]("native-run", "Perform a native run of this project" )
    val testProject = TaskKey[Project]("test-project", "The test sub-project for this project")
    
    val compiler = SettingKey[native.Compiler]("compiler", "The compiler to use for this project")
    
    val modeOption : Parser[String] = token("debug") | token("release")
    val compilerOption : Parser[String] = token("gcc") | token("clang")
    val platformOption  : Parser[String] = token("linux") | token("beaglebone")
    
    val buildOptsParser =
    (
        (Space ~> modeOption) ~
        (Space ~> compilerOption) ~
        (Space ~> platformOption)
    )
    
    def setBuildConfigCommand = Command("set-build-config")(_ => buildOptsParser)
    {
        (state, args) =>
   
        val ((mode, compiler), platform) = args
        val bc = new native.BuildConfig( mode.mkString, compiler.mkString, platform.mkString )
        val extracted : Extracted = Project.extract(state)
        
        // Reconfigure all projects to this new build config
        val buildConfigUpdateCommands = extracted.structure.allProjectRefs.map
        { pref =>
        
            (buildConfig in pref) := Some(bc)
        }
        
        extracted.append( buildConfigUpdateCommands, state )
    }
   
    def buildCommand = Command.command("build")
    { state =>
   
        // TODO: This is NOT HOW IT SHOULD BE. We need to get SBT to use its
        // own internal dependency analysis mechanism here. Yuk yuk yuk.
        val extracted : Extracted = Project.extract(state)
        
        var seenProjectIds = Set[String]()
        def buildProjectRecursively( project : ResolvedProject, projectRef : ProjectRef )
        {
            def scheduleProjectRef( projectReference : ProjectRef ) =
            {
                val resolvedO = Project.getProjectForReference( projectReference, extracted.structure )
                
                
                resolvedO.foreach
                { r =>
                    if ( !seenProjectIds.contains( r.id ) )
                    {
                        seenProjectIds += r.id
                        buildProjectRecursively( r, projectReference )
                    }
                }
            }
            
            project.dependencies.foreach { cpd => scheduleProjectRef( cpd.project ) }
            project.aggregate.foreach { agg => scheduleProjectRef( agg ) }
            
            Project.runTask(
                // The task to run
                nativeCompile in projectRef,
                state,
                // Check for cycles
                true )
        }
        
        buildProjectRecursively( extracted.currentProject, extracted.currentRef )
            
        state
    }
   
    object NativeProject
    {
         def apply(
            id: String,
            base: File,
            aggregate : => Seq[ProjectReference] = Nil,
            dependencies : => Seq[ClasspathDep[ProjectReference]] = Nil,
            delegates : => Seq[ProjectReference] = Nil,
            settings : => Seq[sbt.Project.Setting[_]] = Nil,
            configurations : Seq[Configuration] = Configurations.default ) =
        {
            val defaultSettings = Seq(
            
                commands            ++= Seq( buildCommand, setBuildConfigCommand ),
                
                buildConfig         := None,
            
                compiler            := new native.GccCompiler( file("/usr/bin/g++-4.7"), file("/usr/bin/ar"), file("/usr/bin/g++-4.7") ),
                
                rootBuildDirectory      <<= (baseDirectory, buildConfig) map
                { case (bd, bco) =>
                
                    if ( bco.isEmpty ) error( "Please set a build configuration using set-build-config" )
                    val bc = bco.get
                
                    val dir = bd / "sbtbuild" / bc.platform / bc.compiler / bc.mode
                    
                    IO.createDirectory(dir)
                    
                    dir
                },
                
                projectBuildDirectory      <<= (rootBuildDirectory, name) map
                { case (rbd, n) =>
                
                    val dir = rbd / n
                    
                    IO.createDirectory(dir)
                    
                    dir
                },
                
                stateCacheDirectory <<= (projectBuildDirectory) map { _ / "state-cache"  },
                
                includeDirectories  <<= (projectDirectory) map { pd => Seq(pd / "interface") },
                
                linkDirectories     :=  Seq(),
                
                nativeLibraries     <<= (projectBuildDirectory) map { _ => Seq() },
                
                sourceDirectory     <<= (projectDirectory) map { _ / "source" },
                
                sourceFiles         <<= (sourceDirectory) map { pd => ((pd ** "*.cpp").get ++ (pd ** "*.c").get).toSet },
                
                sourceFilesWithDeps <<= (compiler, projectBuildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, streams) map
                {
                    case (c, bd, scd, ids, sfs, s) =>
                    
                    // Calculate dependencies
                    def findDependencies( sourceFile : File ) : Set[File] =
                    {
                        val depGen = c.findHeaderDependencies( s, bd, ids, sourceFile )
                        
                        depGen.runIfNotCached( scd, Set(sourceFile) )
                        
                        IO.readLines(depGen.resultPath).map( file ).toSet
                    }
                    
                    sfs.map( sf => (sf, findDependencies(sf) ) ).toMap
                },
                
                watchSources        <++= (sourceFilesWithDeps) map { sfd => sfd.toList.flatMap { case (sf, deps) => (sf +: deps.toList) } },
                
                objectFiles         <<= (compiler, projectBuildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, sourceFilesWithDeps, streams) map
                { case (c, bd, scd, ids, sfs, sfdeps, s) =>
                    
                    // Build each source file in turn as required
                    sfs.map
                    { sourceFile =>
                        
                        val dependencies = sfdeps(sourceFile) + sourceFile
                        
                        s.log.debug( "Dependencies for %s: %s".format( sourceFile, dependencies.mkString(";") ) )
                        
                        val blf = c.compileToObjectFile( s, bd, ids, sourceFile )
                        
                        blf.runIfNotCached( scd, dependencies.toSet )
                    }
                }
            )
            
            Project( id, base, aggregate, dependencies, delegates, Defaults.defaultSettings ++ (defaultSettings ++ settings), configurations )
        }
    }
    
    
    object StaticLibrary
    {
        def apply(
            id: String,
            base: File,
            aggregate : => Seq[ProjectReference] = Nil,
            dependencies : => Seq[ClasspathDep[ProjectReference]] = Nil,
            delegates : => Seq[ProjectReference] = Nil,
            settings : => Seq[sbt.Project.Setting[_]],
            configurations : Seq[Configuration] = Configurations.default ) =
        {
            val defaultSettings = Seq(
                nativeCompile <<= (compiler, name, projectBuildDirectory, stateCacheDirectory, objectFiles, streams) map
                { case (c, projName, bd, scd, ofs, s) =>
                
                    val blf = c.buildStaticLibrary( s, bd, projName, ofs )
                    
                    blf.runIfNotCached( scd, ofs )
                },
                (compile in Compile) <<= (compile in Compile) dependsOn (nativeCompile)
                
                
            )
            NativeProject( id, base, aggregate, dependencies, delegates, defaultSettings ++ settings, configurations )
        }
    }
    
    object NativeExecutable
    {
        def apply(
            id: String,
            base: File,
            aggregate : => Seq[ProjectReference] = Nil,
            dependencies : => Seq[ClasspathDep[ProjectReference]] = Nil,
            delegates : => Seq[ProjectReference] = Nil,
            settings : => Seq[sbt.Project.Setting[_]] = Defaults.defaultSettings,
            configurations : Seq[Configuration] = Configurations.default ) =
        {
            val defaultSettings = Seq(
                nativeCompile <<= (compiler, name, projectBuildDirectory, stateCacheDirectory, objectFiles, linkDirectories, nativeLibraries, streams) map
                { case (c, projName, bd, scd, ofs, lds, nls, s) =>
                
                    val blf = c.buildExecutable( s, bd, projName, lds, nls, ofs )
                    
                    blf.runIfNotCached( scd, ofs )
                },
                nativeRun <<= (nativeCompile, streams) map
                { case (nbExe, s) =>
                    
                    val res = nbExe.toString !
                    
                    if ( res != 0 ) error( "Non-zero exit code: " + res.toString )
                },
                (compile in Compile) <<= (compile in Compile) dependsOn (nativeCompile)
            )
            NativeProject( id, base, aggregate, dependencies, delegates, defaultSettings ++ settings, configurations )
        }
    }
    
    object NativeExecutable2
    {
        def apply( _name : String, _projectDirectory : File, _settings : => Seq[sbt.Project.Setting[_]] ) =
        {
            println( "Building: " + _name )
            NativeExecutable( id=_name, base=file("./"),
                settings = _settings ++ Seq(
                    name := _name,
                    projectDirectory := _projectDirectory ) )
        }
    }
    
    object StaticLibrary2
    {
        def apply( _name : String, _projectDirectory : File, _settings : => Seq[sbt.Project.Setting[_]] ) =
        {
            lazy val mainLibrary = StaticLibrary( id=_name, base=file("./"),
                settings = _settings ++ Seq(
                    name := _name,
                    projectDirectory := _projectDirectory ) )
                    
            
            // TODO: This sub-library test infrastructure doesn't seem to be working.
            // Add test in as a set of extra keys into the main build instead
            val testDir = (_projectDirectory / "test")
            println( testDir, testDir.exists )
            if ( testDir.exists )
            {
                val testName = _name + "_test"
                lazy val testLibrary = NativeExecutable2( testName, testDir,
                    Seq(
                    ) ++ _settings )
                testLibrary
            }
            else mainLibrary
        }
    }
        
    lazy val utility = StaticLibrary( id="utility", base=file("./"),
        settings=Seq
        (
            name                := "utility",
            projectDirectory    := file( "./libraries/utility" )
        )
    )
   
    lazy val datastructures = StaticLibrary( id="datastructures", base=file("./"),
        settings=Seq
        (
            name                := "datastructures",
            projectDirectory    := file( "./libraries/datastructures" ),
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    
    lazy val functionalcollections2 = StaticLibrary2(
        "functionalcollections2", file( "./libraries/functionalcollections" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    lazy val functionalcollections = StaticLibrary( id="functionalcollections", base=file("./"),
        settings=Seq
        (
            name                := "functionalcollections",
            projectDirectory    := file( "./libraries/functionalcollections" ),
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    lazy val simple = NativeExecutable2( "simple", file( "./applications/simple" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" ),
            objectFiles         <+= (nativeCompile in utility)
        )
    )
    
    lazy val all = NativeProject( id="all", base=file("."), settings=Seq( projectDirectory := file(".") ) ).aggregate( utility, datastructures, functionalcollections, simple )
    
    
}


