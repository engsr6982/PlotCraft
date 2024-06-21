add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
add_requires(
    "levilamina 0.13.0",
    "more-dimensions 0.4.0",
    "sqlitecpp 3.2.1",
    "legacymoney 0.8.1"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("gen")
    set_default(1)
    set_values(1, 2)

target("PlotCraft") -- Change this to your plugin name.
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_HAS_CXX23=1",
        "PLOT_EXPORTS" -- Export PlotCraft
    )
    add_packages(
        "levilamina",
        "more-dimensions",
        "sqlitecpp",
        "legacymoney"
    )
    add_files("src/**.cpp", "src/**.cc")
    add_includedirs(
        "src",
        "include"
    )
    add_shflags("/DELAYLOAD:bedrock_server.dll") -- To use symbols provided by SymbolProvider.
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    if is_mode("debug") then
        add_defines("DEBUG")
    end

    -- 地皮生成器选择
    if get_config("gen") == 1 then
        add_defines("GEN_1")
    else 
        add_defines("GEN_2")
    end

    add_defines("PLUGIN_NAME=\"PlotCraft\"")
    add_defines("PLUGIN_TITLE=\"§6[§aPlotCraft§6]§r \"")

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local tag = os.iorun("git describe --tags --abbrev=0 --always")
        local major, minor, patch, suffix = tag:match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then
            print("Failed to parse version tag, using 0.0.0")
            major, minor, patch = 0, 0, 0
        end
        local plugin_define = {
            pluginName = target:name(),
            pluginFile = path.filename(target:targetfile()),
            pluginVersion = major .. "." .. minor .. "." .. patch,
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
