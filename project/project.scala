import sbt._
import Keys._

object TestBuild extends NativeBuild
{
    lazy val baseGcc = new native.GccCompiler( file("/usr/bin/g++-4.7"), file("/usr/bin/ar"), file("/usr/bin/g++-4.7") )
    
    override lazy val configurations = Set[native.Environment](
        new native.Environment( "release/Gcc/Linux/PC", baseGcc.copy( compileFlags="-O2 -Wall -Wextra" ) ),
        new native.Environment( "debug/Gcc/Linux/PC", baseGcc.copy( compileFlags="-g -Wall -Wextra" ) )
    )
        
    val utility = StaticLibrary( id="utility", base=file("./"),
        settings=Seq
        (
            name                := "utility",
            projectDirectory    := file( "./libraries/utility" )
        )
    )
   
    val datastructures = StaticLibrary( id="datastructures", base=file("./"),
        settings=Seq
        (
            name                := "datastructures",
            projectDirectory    := file( "./libraries/datastructures" ),
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    
    val functionalcollections2 = StaticLibrary2(
        "functionalcollections2", file( "./libraries/functionalcollections" ),
        Seq
        (
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    val functionalcollections = StaticLibrary( id="functionalcollections", base=file("./"),
        settings=Seq
        (
            name                := "functionalcollections",
            projectDirectory    := file( "./libraries/functionalcollections" ),
            includeDirectories  += file( "./libraries/utility/interface" )
        )
    )
    
    
    // Can then do a dependsOn on this and it'll all magically work
    //lazy val navetasScalaLib = uri("ssh://git@github.lan.ise-oxford.com/Navetas/navetasscalalib.git")
    
    val simple = NativeExecutable2( "simple", file( "./applications/simple" ),
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


