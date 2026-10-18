document.addEventListener(
    'DOMContentLoaded',
    function (e) {

        // Move version number at the bottom.
        document.querySelector("body>div>div:last-child").appendChild(document.querySelector("aside"));

        // Move network session at the bottom if SSID is set.
        if(!!document.getElementById("iwcWifiSsid")?.value) {
          const f = document.getElementById("iwcSys");
          f.parentElement.removeChild(f);
          document.querySelector('form').insertBefore(f, document.querySelector('button[type=submit]'));
        }

        // Add network status banner
        const wifiStatusBanner = document.createElement("div");
        wifiStatusBanner.className = "network-status-banner";
        const hasSsid = !!document.getElementById("iwcWifiSsid")?.value;
        if (hasSsid) {
            wifiStatusBanner.innerHTML = "<span>🟢 Network: Configured</span>";
        } else {
            wifiStatusBanner.innerHTML = "<span>🟠 Network: AP Mode (Not configured)</span>";
        }
        document.querySelector("aside")?.insertAdjacentElement("afterend", wifiStatusBanner);

        // Replace text inputs with desired type
        document.querySelectorAll("[data-type]").forEach((el) => {
          el.type = el.getAttribute("data-type");
        });

        // Add a toggle to reveal values in password fields.
        document.querySelectorAll('input[type=password]').forEach((p) => {
            const btn = document.createElement('input');
            btn.classList.add('pwtoggle');
            btn.type = 'button';
            btn.value = '👁️';
            p.insertAdjacentElement('afterend', btn);
            btn.onclick = function() { if (p.type === 'password') { p.type = 'text'; btn.value = '🔒'; } else { p.type = 'password'; btn.value = '👁️'; } }
        });

        // Disables the submit button and update its label when saving.
        const formEl = document.querySelector("form");
        if (formEl) {
            formEl.addEventListener('submit', () => {
                let btn = document.querySelector("button[type=submit]");
                btn.innerText = "Saving..."; 
                btn.toggleAttribute("disabled", true);
            });   
        }

        //No fear...
        (async () => {
            let done = false;

            // Add placeholder.
            let ssid = document.getElementById("iwcWifiSsid");
            ssid.setAttribute('list', 'apslist');
            var datalist = document.createElement("DATALIST");
            datalist.setAttribute("id", "apslist");
            const formEl = document.querySelector("form");
            formEl.appendChild(datalist);
            // Add entrdatalist.
            var option = document.createElement("option");
            option.value = "Scanning for WiFi networks...";
            datalist.appendChild(option);

            // add a small delay, as scanning affects response of the esp.
            await new Promise(r => setTimeout(r, 300));
            while (!done) {
                // start scan. first result is always empty list.
                const result = await fetch("wifilist");

                if (result.status == 202) {
                    // request accepted, not yet handled.
                    // wait and retry in 1 second.
                    await new Promise(r => setTimeout(r, 1000));
                }
                else if (result.status == 200) {
                    // result is in.
                    done = true;
                    while (datalist.firstChild) {
                        datalist.removeChild(datalist.firstChild)
                    }

                    // Get the result.
                    const str = await result.text();
                    // If not empty, add it.
                    if (str.length > 0) {
                        const array = str.split("\n");
                        // Filter the list.
                        arrayfiltered = array.sort().filter(function (item, pos, ary) {
                            return !pos || item != ary[pos - 1];
                        });
                        arrayfiltered.forEach(function (item, index) {
                            // Do something
                            if (item.length > 0) {
                                var option = document.createElement("option");
                                option.value = item;
                                datalist.appendChild(option);
                            }

                        });
                    }
                } else {
                    // Unexpected error code. bail out.
                    console.log("Invalid response from fetching wifilist.");
                    done = true;
                }
            }
        })();

        // Replaces inputs with data-options attribute with proper select elements.
        // Inspiration: https://github.com/prampec/IotWebConf/issues/29#issuecomment-533751475

        const customSelects = document.querySelectorAll('input[data-options]');

        customSelects.forEach((customSelect) => {
            const defaultValue = customSelect.value;
            const options = customSelect.getAttribute('data-options').split('|');
            const selectEl = document.createElement('select');
            selectEl.name = customSelect.name;
            selectEl.id = customSelect.id;
            if (defaultValue === '') {
                selectEl.appendChild(document.createElement('option'))
            }
            let optgroupEl = null;
            options.forEach((option, index) => {
                const multipart = option.split('/');
                let optionLabel = option;
                if (multipart.length > 1) {
                    const groupLabel = multipart.splice(0,1)[0];
                    // Start new optgroup ?
                    if (!optgroupEl || (groupLabel != optgroupEl.label)) {
                        optgroupEl && selectEl.appendChild(optgroupEl);
                        optgroupEl = document.createElement('optgroup');
                        optgroupEl.label = groupLabel;
                    }
                    optionLabel = multipart.join(" / ");
                } else {
                    // No multipart label, close current group.
                    if (optgroupEl) {
                        selectEl.appendChild(optgroupEl);
                        optgroupEl = null;
                    }
                }
                const optionEl = document.createElement('option');
                optionEl.value = index;
                optionEl.innerText = optionLabel;
                if (index == defaultValue) {
                    optionEl.toggleAttribute('selected');
                }
                if (optgroupEl) {
                    optgroupEl.appendChild(optionEl);
                } else {
                    selectEl.appendChild(optionEl);
                }
            });
            if (optgroupEl) {
                selectEl.appendChild(optgroupEl);
            }
            customSelect.id += '-d';
            const ctrlAttr = customSelect.getAttribute('data-controlledby');
            const ctrlVal = customSelect.getAttribute('data-showon');
            if (ctrlAttr && ctrlVal) {
              selectEl.setAttribute('data-controlledby', ctrlAttr);
              selectEl.setAttribute('data-showon', ctrlVal);
            }
            customSelect.insertAdjacentElement('beforebegin', selectEl);
            customSelect.parentElement.removeChild(customSelect);
        });

        // Validation for AP password requirement
        const apPwdInput = document.getElementById("iwcApPassword");
        const wifiSsidInput = document.getElementById("iwcWifiSsid");
        if (apPwdInput) {
            const apNotice = document.createElement("small");
            apNotice.className = "ap-pwd-notice";
            apNotice.style.display = "block";
            apNotice.style.marginTop = "4px";
            apNotice.style.fontSize = "0.8em";
            apNotice.style.color = "#ffb86d";
            apNotice.innerText = "ℹ️ Required: min. 8 chars to activate Wi-Fi (cannot be default 'password').";
            apPwdInput.parentElement.appendChild(apNotice);

            const validateApPassword = () => {
                const val = apPwdInput.value;
                const hasWifi = wifiSsidInput && wifiSsidInput.value.trim().length > 0;
                if (hasWifi && (val.length < 8 || val === "password")) {
                    apPwdInput.style.borderColor = "#f79292";
                    apNotice.style.color = "#f79292";
                    apNotice.innerText = "⚠️ AP password must be changed (min. 8 chars) or Wi-Fi configuration will not be applied!";
                } else {
                    apPwdInput.style.borderColor = "";
                    apNotice.style.color = "#72e492";
                    apNotice.innerText = "✓ AP password requirements met.";
                }
            };
            apPwdInput.addEventListener("input", validateApPassword);
            if (wifiSsidInput) wifiSsidInput.addEventListener("input", validateApPassword);
        }

        // Adds a label next to Range inputs.
        document.querySelectorAll("input[type=range]").forEach((inputEl) => {
            const labels = inputEl.getAttribute("data-labels");
            const labelList = labels && labels.split("|");
            const setLabel = () => {
                inputEl.setAttribute("data-label", labelList ? labelList[parseInt(inputEl.value, 10)] || inputEl.value : inputEl.value);
            };
            inputEl.oninput = setLabel;
            setLabel();
        });

        // Show/hide fields based on others.
        document.querySelectorAll("[data-controlledby]").forEach((controlledEl) => {
          const controllerEl = document.getElementById(controlledEl.getAttribute('data-controlledby'));
          const showValues = controlledEl.getAttribute('data-showon').split('|');
          const apply = () => {
            controlledEl.parentElement.style.display = showValues.indexOf(controllerEl.value + "") < 0 ? 'none' : '';
          };
          controllerEl.addEventListener('change', apply);
          apply();
        });

        // Apply the selected color to the clock logo :)
        const colorInputEl = document.querySelector('input[type=color]');
        if (colorInputEl) {
            const updateLogoColor = () => {
                document.querySelector('.logoContainer').style.backgroundColor = colorInputEl.value
            };
            colorInputEl.addEventListener('input', updateLogoColor);
            updateLogoColor();
        }

        // MQTT Test Connection button
        const mqttPwdInput = document.getElementById("mqtt_password");
        if (mqttPwdInput) {
            const container = document.createElement("div");
            container.className = "mqtt-test-container";
            container.style.marginTop = "15px";
            container.style.display = "flex";
            container.style.flexDirection = "column";

            const btn = document.createElement("button");
            btn.type = "button";
            btn.className = "mqtt-test-btn";
            btn.innerText = "⚡ Test MQTT Connection";

            const resultDiv = document.createElement("div");
            resultDiv.className = "mqtt-test-result";
            resultDiv.style.display = "none";

            btn.onclick = async function () {
                const serverEl = document.getElementById("mqtt_server");
                const portEl = document.getElementById("mqtt_port");
                const userEl = document.getElementById("mqtt_user");
                const pwdEl = document.getElementById("mqtt_password");

                const server = serverEl ? serverEl.value.trim() : "";
                const port = portEl ? portEl.value.trim() : "1883";
                const user = userEl ? userEl.value.trim() : "";
                const password = pwdEl ? pwdEl.value : "";

                if (!server) {
                    resultDiv.style.display = "block";
                    resultDiv.className = "mqtt-test-result error";
                    resultDiv.innerText = "✗ Please enter an MQTT server address first.";
                    return;
                }

                btn.disabled = true;
                btn.innerText = "Testing connection...";
                resultDiv.style.display = "block";
                resultDiv.className = "mqtt-test-result pending";
                resultDiv.innerText = "Connecting to " + server + ":" + port + "...";

                try {
                    const resp = await fetch("/api/mqtt/test", {
                        method: "POST",
                        headers: { "Content-Type": "application/x-www-form-urlencoded" },
                        body: new URLSearchParams({ server, port, user, password })
                    });
                    const data = await resp.json();
                    if (data.status === "ok") {
                        resultDiv.className = "mqtt-test-result ok";
                        resultDiv.innerText = "✓ " + data.message;
                    } else {
                        resultDiv.className = "mqtt-test-result error";
                        resultDiv.innerText = "✗ " + data.message;
                    }
                } catch (err) {
                    resultDiv.className = "mqtt-test-result error";
                    resultDiv.innerText = "✗ Request failed: " + err.message;
                } finally {
                    btn.disabled = false;
                    btn.innerText = "⚡ Test MQTT Connection";
                }
            };

            container.appendChild(btn);
            container.appendChild(resultDiv);
            mqttPwdInput.parentElement.insertAdjacentElement("afterend", container);

            const applyMqttTestVisibility = () => {
                const mqttCtrl = document.getElementById("mqtt_enabled");
                if (mqttCtrl) {
                    container.style.display = (mqttCtrl.value === "1") ? "flex" : "none";
                }
            };
            const mqttCtrl = document.getElementById("mqtt_enabled");
            if (mqttCtrl) {
                mqttCtrl.addEventListener("change", applyMqttTestVisibility);
                applyMqttTestVisibility();
            }
        }

        // Reveal the form.
        document.body.classList.add('ready');
    }
)