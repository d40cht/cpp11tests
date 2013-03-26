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
    val buildDirectory = SettingKey[File]("build-dir", "Build directory")
    val stateCacheDirectory = TaskKey[File]("state-cache-dir", "Build state cache directory")
    val projectDirectory = SettingKey[File]("project-dir", "Project directory")
    val includeDirectories = TaskKey[Set[File]]("include-dirs", "Include directories")
    val sourceFiles = TaskKey[Set[File]]("source-files", "All source files for this project")
    val objectFiles = TaskKey[Set[File]]("object-files", "All object files for this project" )
    
        
    lazy val foo = Project( id="foo", base=file("."),
        settings=Seq
        (
            // Don't redo work: http://www.scala-sbt.org/release/docs/faq.html#generate-sources-resources
            compilerExe         := new File( "/usr/bin/g++-4.7" ),
            buildDirectory      := new File( "./sbtbuild" ),
            stateCacheDirectory <<= (buildDirectory) map { bd => bd / "state-cache" },
            projectDirectory    := new File( "./libraries/utility" ),
            includeDirectories  <<= (projectDirectory) map { pd => Set(pd / "interface") },
            
            sourceFiles         <<= (projectDirectory) map { pd => ((pd ** "*.cpp").get ++ (pd ** "*.c").get).toSet },
            
            objectFiles         <<= (compilerExe, buildDirectory, stateCacheDirectory, includeDirectories, sourceFiles, streams) map
            { case (ce, bd, scd, ids, sfs, s) =>
                
                val includePathArg = ids.map( ip => "-I " + ip ).mkString(" ")
                def findDependencies( sourceFile : File ) : Set[File] =
                {
                    val depFile = bd / (sourceFile.base + ".d")
                    
                    val lazyBuild = FileFunction.cached( scd / depFile.toString , FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                        val depCmd = "%s -MM -MF %s %s %s".format( ce, depFile, includePathArg, sourceFile )
                        s.log.info( "Executing: " + depCmd )
                        depCmd !!
                        
                        Set(depFile)
                    }
                    
                    lazyBuild(Set(sourceFile))
                    
                    // Strip off any trailing backslash characters from the output
                    val trimmedLines = IO.readLines( depFile ).map( l => new File( l.replace("\\", "" ).split(" ").drop(1).mkString(" ").trim ) )
                    trimmedLines.toSet
                }
                
                sfs.map
                { sourceFile =>
                    
                    val dependencies = findDependencies( sourceFile ) + sourceFile
                    
                    s.log.debug( "Dependencies for %s: %s".format( sourceFile, dependencies.mkString(";") ) )
                    
                    val opFile = bd / (sourceFile.base + ".obj")
                    val lazyBuild = FileFunction.cached( scd / opFile.toString, FilesInfo.lastModified, FilesInfo.exists )
                    { _ =>
                      
                        val buildCmd = "%s %s -g -c -o %s %s".format( ce, includePathArg, opFile, sourceFile )
                       
                        s.log.info( "Executing: " + buildCmd )
                        buildCmd !!
                        
                        Set(opFile)
                    }
                    
                    lazyBuild( dependencies ).head
                }
            }
            
            /*fileToCount := new File( "./foo.txt" ),
            lineCounter <<= (buildDirectory, fileToCount) map
            { case (bd, ftc) =>
         
                val cachedFun = FileFunction.cached( bd / "stateCache", FilesInfo.lastModified, FilesInfo.exists )
                { inFiles : Set[File] =>
                
                    val inFile = inFiles.head
                    val outFile = bd / (inFile.base +  ".linecount")    
                    
                    val lines = IO.readLines( inFile ).size
                    println( "Number of lines: " + lines )
                    
                    
                    IO.write( outFile, lines.toString )
                    
                    Set[File](outFile)
                }
                
                cachedFun( Set(ftc) ).head
            }*/
            /*projectRootDir  := new File("./libraries"),
            
            sourceFiles <<= (projectRootDir) map
            { rootDir =>
                
                val all = rootDir ** "*.cpp"
                
                println( all )
                
                all.get
            }*/
        )
    )
    
}


