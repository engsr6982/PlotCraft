add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
add_requires(
    "levilamina 0.13.5",
    "sqlitecpp 3.2.1",
    "legacymoney 0.8.3"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

if get_config("overworld") == false then 
    add_requires("more-dimensions 0.4.1")
end 

if get_config("remote") == true then
    add_requires("legacyremotecall 0.8.3")
end 

option("gen") -- Generator
    set_default(1)
    set_values(1, 2)

option("overworld") -- Overworld
    set_default(false)

option("remote") -- RemoteCall
    set_default(false)


target("PlotCraft")
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

    -- GEN_1
    -- GEN_2
    -- OVERWORLD
    -- REMOTE_API

    if is_mode("debug") then
        add_defines("DEBUG")
    end

    if get_config("overworld") == false then
        add_packages("more-dimensions")
    else
        add_defines("OVERWORLD")
    end 

    if get_config("remote") == true  then
        add_packages("legacyremotecall")
        add_defines("REMOTE_API")
    end

    if get_config("gen") == 1 then
        add_defines("GEN_1")
    else 
        add_defines("GEN_2")
    end

    add_defines("BuildVersionInfo=\"PlotGenerator: " .. tostring(get_config("gen")) .. " | Overworld: " .. tostring(get_config("overworld")) .. " | RemoteCall: " .. tostring(get_config("remote")) .. "\"")

    add_defines("PLUGIN_NAME=\"PlotCraft\"")
    add_defines("PLUGIN_TITLE=\"§6[§aPlotCraft§6]§r \"")

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        cprint("${bright green}[Build Infomation]: ${reset}PlotGenerator: " .. tostring(get_config("gen")) .. " | Overworld: " .. tostring(get_config("overworld")) .. " | RemoteCall: " .. tostring(get_config("remote")))

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
