local logger = {}

function logger.info(msg, console)
    spdlog.info("[INFO   ] " .. msg)
    if console then
        print("[IRF][INFO   ] " .. msg)
    end
end

function logger.warn(msg, console)
    spdlog.info("[WARNING] " .. msg)
    if console then
        print("[IRF][WARNING] " .. msg)
    end
end

function logger.error(msg, console)
    spdlog.info("[ERROR  ] " .. msg)
    if console then
        print("[IRF][ERROR  ] " .. msg)
    end
end

return logger