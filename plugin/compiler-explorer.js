(function () {
    function init() {
        const ce_nodes = document.querySelectorAll('code.language-cpp');
       
        for (let i = 0, len = ce_nodes.length; i < len; i++) {
            let element = ce_nodes[i];
            let compiler = "g8";
            let options = "-Og -march=native -std=c++17";
            let source = unescape(element.textContent);
            let lines = source.split('\n');
            source = "";
            let displaySource = "";
            const configMatcher = /^\s*\/\/\/\s*([^:]+):(.*)$/;
            const hideMatcher = /^\s*\/\/\/\s*((un)?hide)$/;
            let skipDisplay = false;
            let hide = false;
            for (let idx = 0; idx < lines.length; ++idx) {
                let line = lines[idx];
                let match = line.match(configMatcher);
                if (match) {
                    compiler = match[1];
                    options = match[2];
                } else {
                    match = line.match(hideMatcher);
                    if (match) {
                        hide = match[1] === "hide";
                        console.log(line);
                        console.log(hide);
                        continue;
                    }
                    if (line === "// setup") {
                        skipDisplay = true;
                    } else if (line[0] !== ' ') {
                        skipDisplay = false;
                    }

                    source += line + "\n";
                    if (!skipDisplay && !hide)
                        displaySource += line + "\n";
                    if (line.length > 36) {
                        console.error(`Line too long: "${line}"`);
                    }
                }
            }

            function trim(source) {
                while (source.startsWith("\n")) {
                    source = source.slice(1, source.length);
                }
                while (source.endsWith("\n\n")) {
                    source = source.slice(0, source.length - 1);
                }
                return source;
            }

            displaySource = trim(displaySource);
            source = trim(source);
            options += " -Wall -Wextra -pedantic";
            let content = [];
            content.push({
                type: 'component',
                componentName: 'codeEditor',
                componentState: {
                    id: 1,
                    source: source,
                    options: {compileOnChange: true, colouriseAsm: true},
                    fontScale: 1
                }
            });
            content.push({
                type: 'component',
                componentName: 'compiler',
                componentState: {
                    source: 1,
                    filters: {commentOnly: true, directives: true, intel: true, labels: true, trim: true, execute: true},
                    options: options,
                    compiler: compiler,
                    fontScale: 1.0
                }
            });

            content.push({
                type: 'component',
                componentName: 'output',
                componentState: {
                    source: 1,
                    compiler: 1,
                }
            });
            
            let obj = {
                version: 4,
                content: [{type: 'row', content: content}]
            };
            let ceFragment = encodeURIComponent(JSON.stringify(obj));

            const isLocal = !!window.location.host.match(/localhost/gi);
            const baseUrl = isLocal ? 'http://localhost:10240/' : 'https://godbolt.org/';

            element.addEventListener('click', (evt) => {
                if (evt.shiftKey) {
                    window.location.assign(baseUrl + "#" + ceFragment);
                }
                if (evt.ctrlKey) {
                    window.open(baseUrl + "#" + ceFragment, '_blank');
                }
            });
            element.textContent = displaySource;
        }
    }

    const CePlugin = {init};
    Reveal.registerPlugin('compiler-explorer', CePlugin);
})();
