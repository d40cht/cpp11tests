import sbt._
import Keys._

object TestBuild extends Build
{
    val gccOptions = TaskKey[Seq[String]]("gcc-options", "Options to pass to gcc")
    val archiveOptions = TaskKey[Seq[String]]("archive-options", "Options to pass to ar")
    val projectRootDir = TaskKey[File]("project-root", "Project root directory")
    //val sourceFiles = TaskKey[Seq[File]]("source-files", "")
    //val objectFiles = TaskKey[Seq[File]]("object-files", "")
    //val archive = TaskKey[File]("object", "Object files input to archive")
    
    //val fileToCount = TaskKey[File]("file-to-count", "File whose number of lines we count")
    //val lineCounter = TaskKey[File]("line-counter", "Count lines in files")
    
    val compilerExe = SettingKey[File]("compiler", "Path to compiler executable")
    val archiveExe = SettingKey[File]("archive", "Path to archive executable")
    val buildDirectory = TaskKey[File]("build-dir", "Build directory")
    val stateCacheDirectory = TaskKey[File]("state-cache-dir", "Build state cache directory")
    val projectDirectory = SettingKey[File]("project-dir", "Project directory")
    val sourceDirectory = TaskKey[File]("source-dir", "Source directory")
    val includeDirectories = TaskKey[Set[File]]("include-dirs", "Include directories")
    val nativeLibraries = TaskKey[Set[File]]("native-libraries", "All native library dependencies for this project")
    val sourceFiles = TaskKey[Set[File]]("source-files", "All source files for this project")
    val sourceFilesWithDeps = TaskKey[Map[File, Set[File]]]("source-files-with-deps", "All source files for this project")
    val objectFiles = TaskKey[Set[File]]("object-files", "All object files for this project" )
    val nativeBuild = TaskKey[Unit]("native-build", "Perform a native build for this project" )
    val testProject = TaskKey[Option[Project]]("test-project", "The test sub-project for this project")
   
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
            
                compilerExe         := new File( "/usr/bin/g++-4.7" ),
                
                archiveExe          := new File( "/usr/bin/ar" ),
                
                buildDirectory      <<= (baseDirectory, name) map
                { case (bd, n) =>
                    val dir = bd / "sbtbuild" / n
                    
                    IO.createDirectory(dir)
                    
                    dir
                },
                
                stateCacheDirectory <<= (buildDirectory) map { _ / "state-cache"  },
                
                includeDirectories  <<= (projectDirectory) map { pd => Set(pd / "interface") },
                
                nativeLibraries     <<= (buildDirectory) map { _ => Set() },
                
                sourceDirectory     <<= (projectDirectory) map { _ / "source" },
                
                sourceFiles         <<= (sourceDirectory) map { pd => ((pd ** "*.cpp").get ++ (pd ** "*.c").get).toSet },
                
                sourceFilesWithDeps <<= (compilerExe, buildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, streams) map
                {
                    case (ce, bd, scd, ids, sfs, s) =>
                    
                    // Calculate dependencies
                    val includePathArg = ids.map( ip => "-I " + ip ).mkString(" ")
                    def findDependencies( sourceFile : File ) : Set[File] =
                    {
                        val depFile = bd / (sourceFile.base + ".d")
                        
                        val lazyBuild = FileFunction.cached( scd / depFile.toString , FilesInfo.lastModified, FilesInfo.exists )
                        { _ =>
                            val depCmd = "%s -std=c++11 -M -MF %s %s %s".format( ce, depFile, includePathArg, sourceFile )
                            s.log.info( "Executing: " + depCmd )
                            depCmd !!
                            
                            Set(depFile)
                        }
                        
                        lazyBuild(Set(sourceFile))
                        
                        // Strip off any trailing backslash characters from the output
                        val depFileLines = IO.readLines( depFile ).map( _.replace( "\\", "" ) )
                        
                        // Drop the first column and split on spaces to get all the files (potentially several per line )
                        val allFiles = depFileLines.flatMap( _.split(" ").drop(1) ).map( x => new File(x.trim) )
                        allFiles.toSet
                    }
                    
                    sfs.map( sf => (sf, findDependencies(sf) ) ).toMap
                },
                
                watchSources        <++= (sourceFilesWithDeps) map { sfd => sfd.toList.flatMap { case (sf, deps) => (sf +: deps.toList) } },
                
                objectFiles         <<= (compilerExe, buildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, sourceFilesWithDeps, streams) map
                { case (ce, bd, scd, ids, sfs, sfdeps, s) =>
                    
                    // Calculate dependencies
                    val includePathArg = ids.map( ip => "-I " + ip ).mkString(" ")
                    
                    // Build each source file in turn as required
                    sfs.map
                    { sourceFile =>
                        
                        val dependencies = sfdeps(sourceFile) + sourceFile
                        
                        s.log.debug( "Dependencies for %s: %s".format( sourceFile, dependencies.mkString(";") ) )
                        
                        val opFile = bd / (sourceFile.base + ".o")
                        val lazyBuild = FileFunction.cached( scd / opFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                        { _ =>  
                          
                            val buildCmd = "%s -std=c++11 -Werror -Wall %s -g -c -o %s %s".format( ce, includePathArg, opFile, sourceFile )
                           
                            s.log.info( "Executing: " + buildCmd )
                            buildCmd !!
                            
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
                nativeBuild           <<= (name, archiveExe, buildDirectory, stateCacheDirectory, objectFiles, streams) map
                { case (projName, are, bd, scd, ofs, s) =>
                
                    val arFile = bd / (projName + ".a")
                    val lazyBuild = FileFunction.cached( scd / arFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                        val arCmd = "%s -c -r %s %s".format( are, arFile, ofs.mkString(" ") )
                        s.log.info( "Executing: " + arCmd )
                        arCmd !!
                        
                        Set(arFile)
                    }
                    lazyBuild( ofs )
                    
                    arFile
                },
                (packageBin in Compile) <<= (packageBin in Compile) dependsOn (nativeBuild)
                
                
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
                nativeBuild           <<= (name, compilerExe, buildDirectory, stateCacheDirectory, objectFiles, nativeLibraries, streams) map
                { case (projName, cxe, bd, scd, ofs, nls, s) =>
                
                    val exeFile = bd / projName
                    val lazyBuild = FileFunction.cached( scd / exeFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                        val linkCmd = "%s -o %s %s %s".format( cxe, exeFile, ofs.mkString(" "), nls.mkString(" ") )
                        s.log.info( "Executing: " + linkCmd )
                        linkCmd !!
                        
                        Set(exeFile)
                    }
                    lazyBuild( ofs )
                    
                    exeFile
                },
                (packageBin in Compile) <<= (packageBin in Compile) dependsOn (nativeBuild)
            )
            NativeProject( id, base, aggregate, dependencies, delegates, defaultSettings ++ settings, configurations )
        }
    }
    
    
    
        
    lazy val utility = StaticLibrary( id="utility", base=file("./"),
        settings=Seq
        (
            name                := "utility",
            projectDirectory    := new File( "./libraries/utility" )
        )
    )
   
    lazy val datastructures = StaticLibrary( id="datastructures", base=file("./"),
        settings=Seq
        (
            name                := "datastructures",
            projectDirectory    := new File( "./libraries/datastructures" ),
            includeDirectories  += new File( "./libraries/utility/interface" )
        )
    )
    
    lazy val functionalcollections = StaticLibrary( id="functionalcollections", base=file("./"),
        settings=Seq
        (
            name                := "functionalcollections",
            projectDirectory    := new File( "./libraries/functionalcollections" ),
            includeDirectories  += new File( "./libraries/utility/interface" )
        )
    )
    
    lazy val simple = NativeExecutable( id="simple", base=file("./"),
        settings=Seq
        (
            name                := "simple",
            projectDirectory    := new File( "./applications/simple" ),
            includeDirectories  += new File( "./libraries/utility/interface" ),
            nativeLibraries     ++= Set( new File( "./sbtbuild/utility/utility.a" ), new File( "./sbtbuild/functionalcollections/functionalcollections.a" ), new File( "./sbtbuild/datastructures/datastructures.a" ) )
        )
    ).dependsOn( datastructures, utility, functionalcollections )
    
    lazy val all = Project( id="all", base=file(".") ).aggregate( utility, datastructures, functionalcollections, simple )
    
    
}


