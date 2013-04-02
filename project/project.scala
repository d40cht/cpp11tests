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
        
    val utility = StaticLibrary(
        id="utility", base=file("./"),
        settings=Seq
        (
            name                := "utility",
            projectDirectory    := file( "./libraries/utility" )
        )
    )
   
    val datastructures = StaticLibrary2( 
        "datastructures", file( "./libraries/datastructures" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    
    val functionalcollections = StaticLibrary2(
        "functionalcollections", file( "./libraries/functionalcollections" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    
    // Can then do a dependsOn on this and it'll all magically work
    //lazy val navetasScalaLib = uri("ssh://git@github.lan.ise-oxford.com/Navetas/navetasscalalib.git")
    
    val simple = NativeExecutable2(
        "simple", file( "./applications/simple" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" ),
            objectFiles         <+= (nativeCompile in utility)
            //linkerInputs <+= (staticLibrary in utility)
        )
    )
    
    // Since preventing the import of scala default settings, there is no compile on a raw nativeproject and so aggregate is a bit useless
    val all = NativeProject( id="all", base=file("."), settings=Seq( projectDirectory := file(".") ) ).aggregate( utility, datastructures, functionalcollections, simple )
    
    
}


