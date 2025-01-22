add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("miracleforest-repo https://github.com/MiracleForest/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
if is_config("target_type", "server") then
    add_requires("levilamina 1.0.0-rc.3", {configs = {target_type = "server"}})
else
    add_requires("levilamina 1.0.0-rc.3", {configs = {target_type = "client"}})
end
add_requires("levibuildscript")
add_requires("ilistenattentively 0.2.2")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

if get_config("overworld") == false then 
    add_requires("more-dimensions 0.4.1")
end 

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("overworld") -- Overworld
    set_default(false)

target("PlotCraft")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
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
        "ilistenattentively"
    )
    add_files("src/**.cpp", "src/**.cc")
    add_includedirs(
        "src",
        "include"
    )
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    if get_config("overworld") == false then
        add_packages("more-dimensions")
    else
        add_defines("OVERWORLD")
    end

    add_defines("BuildVersionInfo=\"Overworld: " .. tostring(get_config("overworld")) .. "\"")

    add_defines("PLUGIN_NAME=\"PlotCraft\"")
    add_defines("PLUGIN_TITLE=\"§6[§aPlotCraft§6]§r \"")

    if is_mode("release") then
        add_cxflags("/O2")
    elseif is_mode("debug") then
        add_defines("DEBUG")
    end

    after_build(function (target)
        local bindir = path.join(os.projectdir(), "bin")
        local outputdir = path.join(bindir, target:name())
        local langdir = path.join(os.projectdir(), "assets", "lang")
        os.cp(langdir, outputdir)
    end)
