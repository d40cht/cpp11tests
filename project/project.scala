import sbt._
import Keys._

package native
{
    import sbt.std.{TaskStreams}

    
    trait Compiler
    {
        def findHeaderDependencies( s : TaskStreams[_],  includePaths : Seq[File], sourceFile : File ) : Set[File]
        def compileToObjectFile( s : TaskStreams[_], includePaths : Seq[File], sourceFile : File, outputFile : File ) : Unit
        def buildStaticLibrary( s : TaskStreams[_], objectFiles : Set[File], outputFile : File ) : Unit
        def buildExecutable( s : TaskStreams[_], linkPaths : Seq[File], linkLibraries : Seq[String], inputFiles : Set[File], outputFile : File ) : Unit
    }
    
    class GccCompiler( val compilerPath : File, val archiverPath : File, val linkerPath : File ) extends Compiler
    {
        def findHeaderDependencies( s : TaskStreams[_], includePaths : Seq[File], sourceFile : File ) : Set[File] =
        {
            val includePathArg = includePaths.map( ip => "-I " + ip ).mkString(" ")
            val depCmd = "%s -std=c++11 -M %s %s".format( compilerPath, includePathArg, sourceFile )
            s.log.info( "Executing: " + depCmd )
            val depResult = stringToProcess( depCmd ).lines
            
            // Strip off any trailing backslash characters from the output
            val depFileLines = depResult.map( _.replace( "\\", "" ) )
        
            // Drop the first column and split on spaces to get all the files (potentially several per line )
            val allFiles = depFileLines.flatMap( _.split(" ").drop(1) ).map( x => new File(x.trim) )
            
            allFiles.toSet
        }
        
        def compileToObjectFile( s : TaskStreams[_], includePaths : Seq[File], sourceFile : File, outputFile : File ) : Unit =
        {
            val includePathArg = includePaths.map( ip => "-I " + ip ).mkString(" ")
            val buildCmd = "%s -std=c++11 -Werror -Wall %s -g -c -o %s %s".format( compilerPath, includePathArg, outputFile, sourceFile )
                           
            s.log.info( "Executing: " + buildCmd )
            buildCmd !!
            
            outputFile
        }
        
        def buildStaticLibrary( s : TaskStreams[_], objectFiles : Set[File], outputFile : File ) : Unit =
        {
            val arCmd = "%s -c -r %s %s".format( archiverPath, outputFile, objectFiles.mkString(" ") )
            s.log.info( "Executing: " + arCmd )
            arCmd !!
            
            outputFile
        }
        
        def buildExecutable( s : TaskStreams[_], linkPaths : Seq[File], linkLibraries : Seq[String], inputFiles : Set[File], outputFile : File ) : Unit =
        {
            val linkPathArg = linkPaths.map( lp => "-L " + lp ).mkString(" ")
            val linkCmd = "%s -o %s %s %s %s".format( linkerPath, outputFile, inputFiles.mkString(" "), linkPathArg, linkLibraries.mkString(" ") )
            s.log.info( "Executing: " + linkCmd )
            linkCmd !!
            
            outputFile
        }
    }
}

object TestBuild extends Build
{   
    //val compilerExe = SettingKey[File]("compiler", "Path to compiler executable")
    //val archiveExe = SettingKey[File]("archive", "Path to archive executable")
    val buildDirectory = TaskKey[File]("build-dir", "Build directory")
    val stateCacheDirectory = TaskKey[File]("state-cache-dir", "Build state cache directory")
    val projectDirectory = SettingKey[File]("project-dir", "Project directory")
    val sourceDirectory = TaskKey[File]("source-dir", "Source directory")
    val includeDirectories = TaskKey[Seq[File]]("include-dirs", "Include directories")
    val linkDirectories = TaskKey[Seq[File]]("link-dirs", "Link directories")
    val nativeLibraries = TaskKey[Seq[String]]("native-libraries", "All native library dependencies for this project")
    val sourceFiles = TaskKey[Set[File]]("source-files", "All source files for this project")
    val sourceFilesWithDeps = TaskKey[Map[File, Set[File]]]("source-files-with-deps", "All source files for this project")
    val objectFiles = TaskKey[Set[File]]("object-files", "All object files for this project" )
    val nativeBuild = TaskKey[Unit]("native-build", "Perform a native build for this project" )
    val testProject = TaskKey[Option[Project]]("test-project", "The test sub-project for this project")
    
    val compiler = SettingKey[native.Compiler]("compiler", "The compiler to use for this project")
   
    object NativeProject
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
            
                compiler            := new native.GccCompiler( file("/usr/bin/g++-4.7"), file("/usr/bin/ar"), file("/usr/bin/g++-4.7") ),
                
                buildDirectory      <<= (baseDirectory, name) map
                { case (bd, n) =>
                    val dir = bd / "sbtbuild" / n
                    
                    IO.createDirectory(dir)
                    
                    dir
                },
                
                stateCacheDirectory <<= (buildDirectory) map { _ / "state-cache"  },
                
                includeDirectories  <<= (projectDirectory) map { pd => Seq(pd / "interface") },
                
                linkDirectories     :=  Seq(),
                
                nativeLibraries     <<= (buildDirectory) map { _ => Seq() },
                
                sourceDirectory     <<= (projectDirectory) map { _ / "source" },
                
                sourceFiles         <<= (sourceDirectory) map { pd => ((pd ** "*.cpp").get ++ (pd ** "*.c").get).toSet },
                
                sourceFilesWithDeps <<= (compiler, buildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, streams) map
                {
                    case (c, bd, scd, ids, sfs, s) =>
                    
                    // Calculate dependencies
                    def findDependencies( sourceFile : File ) : Set[File] =
                    {
                        val depFile = bd / (sourceFile.base + ".d")
                        val lazyBuild = FileFunction.cached( scd / depFile.toString , FilesInfo.lastModified, FilesInfo.exists )
                        { _ =>
                        
                            val deps = c.findHeaderDependencies( s, ids, sourceFile )
                            
                            IO.write( depFile, deps.mkString("\n") )
                            
                            Set(depFile)
                        }
                        
                        IO.readLines(depFile).map( file ).toSet
                    }
                    
                    sfs.map( sf => (sf, findDependencies(sf) ) ).toMap
                },
                
                watchSources        <++= (sourceFilesWithDeps) map { sfd => sfd.toList.flatMap { case (sf, deps) => (sf +: deps.toList) } },
                
                objectFiles         <<= (compiler, buildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, sourceFilesWithDeps, streams) map
                { case (c, bd, scd, ids, sfs, sfdeps, s) =>
                    
                    // Build each source file in turn as required
                    sfs.map
                    { sourceFile =>
                        
                        val dependencies = sfdeps(sourceFile) + sourceFile
                        
                        s.log.debug( "Dependencies for %s: %s".format( sourceFile, dependencies.mkString(";") ) )
                        
                        val opFile = bd / (sourceFile.base + ".o")
                        val lazyBuild = FileFunction.cached( scd / opFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                        { _ =>
                        
                            c.compileToObjectFile( s, ids, opFile, sourceFile )
                            Set(opFile)
                        }
                        
                        lazyBuild( dependencies ).head
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
                nativeBuild           <<= (compiler, name, buildDirectory, stateCacheDirectory, objectFiles, streams) map
                { case (c, projName, bd, scd, ofs, s) =>
                
                    val arFile = bd / (projName + ".a")
                    val lazyBuild = FileFunction.cached( scd / arFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                    
                        c.buildStaticLibrary( s, ofs, arFile )
                        
                        Set(arFile)
                    }
                    lazyBuild( ofs )
                    
                    arFile
                },
                (compile in Compile) <<= (compile in Compile) dependsOn (nativeBuild)
                
                
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
                nativeBuild           <<= (compiler, name, buildDirectory, stateCacheDirectory, objectFiles, linkDirectories, nativeLibraries, streams) map
                { case (c, projName, bd, scd, ofs, lds, nls, s) =>
                
                    val exeFile = bd / projName
                    val lazyBuild = FileFunction.cached( scd / exeFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                    
                        c.buildExecutable( s, lds, nls, ofs, exeFile )
                        
                        Set(exeFile)
                    }
                    lazyBuild( ofs )
                    
                    exeFile
                },
                (compile in Compile) <<= (compile in Compile) dependsOn (nativeBuild)
            )
            NativeProject( id, base, aggregate, dependencies, delegates, defaultSettings ++ settings, configurations )
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
    
    lazy val functionalcollections = StaticLibrary( id="functionalcollections", base=file("./"),
        settings=Seq
        (
            name                := "functionalcollections",
            projectDirectory    := file( "./libraries/functionalcollections" ),
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    lazy val simple = NativeExecutable( id="simple", base=file("./"),
        settings=Seq
        (
            name                := "simple",
            projectDirectory    := file( "./applications/simple" ),
            includeDirectories  += file( "./libraries/utility/interface" ),
            linkDirectories     ++= Seq( file("./sbtbuild/utility")  ),
            nativeLibraries     ++= Seq( "utility.a" )
        )
    ).dependsOn( utility )
    
    lazy val all = Project( id="all", base=file(".") ).aggregate( utility, datastructures, functionalcollections, simple )
    
    
}


