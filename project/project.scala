import sbt._
import Keys._
import org.seacourt.build._

object TestBuild extends NativeBuild
{
    lazy val baseGcc = new GccCompiler( file("/usr/bin/g++-4.7"), file("/usr/bin/ar"), file("/usr/bin/g++-4.7") )
    
    override lazy val configurations = Set[Environment](
        new Environment( "release/Gcc/Linux/PC", baseGcc.copy( compileFlags="-std=c++11 -O2 -Wall -Wextra" ) ),
        new Environment( "debug/Gcc/Linux/PC", baseGcc.copy( compileFlags="-std=c++11 -g -Wall -Wextra" ) )
    )
        
    val utility = StaticLibrary( "utility", file( "./libraries/utility" ), Seq() )
        .register()
   
    val datastructures = StaticLibrary( "datastructures", file( "./libraries/datastructures" ), Seq() )
        .nativeDependsOn( utility )
        .register()
    
    val functionalcollections = StaticLibrary( "functionalcollections", file( "./libraries/functionalcollections" ), Seq() )
        .nativeDependsOn( utility )
        .register()
   
    val simple = NativeExecutable( "simple", file( "./applications/simple" ), Seq() )
        .nativeDependsOn( utility )
        .register()
}


