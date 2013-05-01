import sbt._
import Keys._
import org.seacourt.build._

object TestBuild extends NativeDefaultBuild
{
        
    val utility = StaticLibrary( "utility", file( "./libraries/utility" ), Seq() )
   
    val datastructures = StaticLibrary( "datastructures", file( "./libraries/datastructures" ), Seq() )
        .nativeDependsOn( utility )
    
    val functionalcollections = StaticLibrary( "functionalcollections", file( "./libraries/functionalcollections" ), Seq() )
        .nativeDependsOn( utility )
   
    val simple = NativeExecutable( "simple", file( "./applications/simple" ), Seq() )
        .nativeDependsOn( utility )
}


