# Icon Count Editor
A C++ library for editing icon counts in Geometry Dash.

**Note: This library is only for use in mods created with the [Geode SDK](https://github.com/geode-sdk/geode).**

## Inclusion
If you have CPM, you can include this library by adding the following to your `CMakeLists.txt`:
```cmake
CPMAddPackage("gh:hiimjasmine00/icon-count-editor#commit-hash")

target_link_libraries(${PROJECT_NAME} icon-count-editor)
```

## License
This project is licensed under the [MIT License](./LICENSE).