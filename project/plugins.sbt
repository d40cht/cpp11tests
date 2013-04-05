resolvers +=  Resolver.url("SBT cpp repo", url("https://raw.github.com/d40cht/sbt-cpp/master/releases/"))( Patterns("[organisation]/[module]_[scalaVersion]_[sbtVersion]/[revision]/[artifact]-[revision].[ext]") )

addSbtPlugin("org.seacourt.build" % "sbt-cpp" % "0.0.2")

