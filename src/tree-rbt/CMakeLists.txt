cmake_minimum_required(VERSION 3.0)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

project(List C)
# add_executable(tree-rbt tree-rbt.c tree-rbt.h)
add_library(tree-rbt SHARED tree-rbt.c tree-rbt.h)

install(
	TARGETS tree-rbt
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)

install(
	FILES tree-rbt.h
	DESTINATION include
)

# Ajout d'un fichier de configuration de type pkgconfig. Copie le 1er argument vers le 2ème. @ONLY = restreint le remplacement de variable dans tree-rbt.pc.in
# à celles qui ont le format @<var>@ pour éviter les conflits avec la syntaxe CMake ${<var>}.
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/tree-rbt.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/tree-rbt.pc
	@ONLY
)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/tree-rbt.pc
	DESTINATION share/pkgconfig
	COMPONENT "PkgConfig"
)

#  Ajout d'un fichier de configuration de type cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/rbtTreeConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/rbtTreeConfig.cmake
	INSTALL_DESTINATION cmake
)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/rbtTreeConfig.cmake
	DESTINATION cmake
)

# Ajout d'un exécutable dépendant également de tree-rbt.h
add_executable(test-tree-rbt test-tree-rbt.c tree-rbt.h)
# Précision de l'ordre de construction: le programme de test doit se faire
# après la librairie
add_dependencies(test-tree-rbt tree-rbt)
# Aggrégation du programme de test avec la librairie tree-rbt
target_link_libraries(test-tree-rbt tree-rbt)

# Activation des tests
enable_testing()
# Ajout d'un test
add_test(test-tree-rbt ./test-tree-rbt)
