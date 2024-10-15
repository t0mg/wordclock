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

        // Reveal the form.
        document.body.classList.add('ready');
    }
)