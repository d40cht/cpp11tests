resolvers +=  Resolver.url("SBT cpp repo", url("https://raw.github.com/d40cht/sbt-cpp/master/releases/"))( Patterns("[organisation]/[module]_[scalaVersion]_[sbtVersion]/[revision]/[artifact]-[revision].[ext]") )

//resolvers +=  Resolver.file("SBT cpp repo", file("/home/alex/Devel/AW/sbt-cpp/releases"))( Patterns("[organisation]/[module]_[scalaVersion]_[sbtVersion]/[revision]/[artifact]-[revision].[ext]") )

addSbtPlugin("org.seacourt.build" % "sbt-cpp" % "0.0.16")

