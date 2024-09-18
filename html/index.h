#ifndef INDEX_H
#define INDEX_H

#include <Arduino.h>

// #ifdef  TEMPLATE_PLACEHOLDER
   // #undef  TEMPLATE_PLACEHOLDER
// #endif
// #define TEMPLATE_PLACEHOLDER '$'

namespace html
{
  constexpr char content[] PROGMEM = R"%(<!DOCTYPE html>
  <html lang="fr">
    <head>
      <meta charset="utf-8" />
      <style>
        body {
          width: 80%;
          margin: auto;
        }
        #screen {
          border: 1px solid grey;
        }
        #ota.disabled {
          display: none;
        }
        #ota.enabled {
          backdrop-filter: blur(5px);
          color: white;
          display: block;
          position: fixed;
          top: 20px;
          bottom: 20px;
          left: 20px;
          right: 20px;
          z-index: 99999;
          background: #000000aa;
        }
        #ota h2 {
          padding: 20px;
        }
        #ota_text {
          display: block;
          text-align: center;
        }
        #ota_progress {
          width: calc(100% - 40px);
          margin: 20px;
          heigth: 20px;
          border: 1px solid #ededf0;
        }
      </style>
      <script src="https://cdn.jsdelivr.net/npm/chart.js">
      </script>
      <script>

      (function () {
        "use strict";
        window.Magnac = {};
        const byId = document.getElementById.bind(document);
        function bitswap (b) {
          b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
          b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
          b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
          return b;
        }
        
        let ota_refresh = false;

        document.addEventListener("DOMContentLoaded", () =>{ 
          const labels = ["24h", "1h", "15min"];
          Magnac.prix = 2874;
          Magnac.chtconso = {};

          labels.forEach(function (label) {
            Magnac.chtconso[label] = new Chart(
              byId("pltconso_" + label), {
                type: "line",
                data: [],
                options: {
                  scales: {
                    y: {
                      suggestedMin: 0,
                      title: {
                        display: true,
                        text: "Puissance (W)"
                      }
                    },
                  },
                },
              });
          });
          
          function rotateArray(arr, ix) {
            const left = arr.slice(0, ix);
            const right = arr.slice(ix, arr.length);
            return right.concat(left);
          }

          function hoursLabels(size, res) {
            const now = new Date;
            res *= 1000;
            return [...Array(size).keys()].map(function(i) {
              const d = new Date;
              d.setTime(now.getTime() + (i - size) * res);
              return d.toLocaleString("fr-FR")
                .split(" ")[1];
            });
          }

          function updateOta() {
            fetch("/ota").then(function (r) {
              if (!r.ok)
                throw new Error("ota HTTP " + r.status);
              return r.json();
            }).then(function (j) {
              Magnac.ota = j;
              byId("last_boot").innerHTML =
                new Date(j.last_boot * 1000)
                  .toLocaleString("fr-FR");
              byId("ota_progress").setAttribute(
                "value", j.progress);
              byId("ota_text").textContent =
                j.progress + " %";
              if (j.updating) {
                ota_refresh = true;
                byId("ota").classList
                  .toggle("disabled", false);
                byId("ota").classList
                  .toggle("enabled", true);
              }
              else if (ota_refresh)
                location.reload();
            });
          }

          function updateWatts() {
            fetch("/watts").then(function (r) {
              if (!r.ok)
                throw new Error("watts HTTP " + r.status);
              return r.json();
              }).then(function (j) {
                Magnac.watts = j;
                byId("p1").innerHTML =
                  -Math.round(j.power1) + " W";

                byId("p2").innerHTML =
                  Math.round(j.power2) + " W";

                byId("ptot").innerHTML =
                  Math.round(j.power2 - j.power1) + " W";
              });
          }

          function updateDimmer() {
            fetch("/dimmer").then(r => {
              if (!r.ok)
                throw new Error("dimmer HTTP " + r.status);
              return r.json();
            }).then(function (j) {
              Magnac.dimmer = j;
            });
          }

          function drawScreen(bin) {
            // console.log(bin);
          }

          const ctx = byId("screen").getContext("2d");
          ctx.font = "28px sans-serif";
          ctx.fillText("RBR Magnac", 24, 36);
          function updateScreen() {
            fetch("/screen").then(r => {
              if (!r.ok)
                throw new Error("screen HTTP " + r.status);
              return r.json();
            }).then(function (j) {
              Magnac.screen = j;
              // TODO decode 64
              console.log(j.xbmp64)
              drawScreen(atob(j.xbmp64));
            });
          }

          function integrate(yy, dx) {
            return yy.reduce((a, y) => a + y * dx);
          }

          const onlyPos = arr =>
            arr.map(x => Math.max(x, 0));

          byId("ekWh").addEventListener("change", () => {
            Magnac.prix = parseInt(this.value);
            labels.forEach(updateData);
          });

          function updateData(label) {
            fetch("/data_" + label).then(r => {
              if (!r.ok)
                throw new Error(`${label} HTTP${r.status}`);
              return r.json();
            }).then(function (j) {
              Magnac.data = { label: j };
              const p1 = j.p1.map(x => -x);

              const conso = onlyPos(j.p2);
              const cumul = integrate(conso, j.res) /3600e3;
              const euro  = cumul * Magnac.prix / 1e4;
              byId("cumul_" + label).innerHTML = 
                `${cumul.toFixed(3)} kWh
                &times 0.${Magnac.prix} € / kWh =
                ${euro.toFixed(2)} €`;

              const eco = onlyPos(p1);
              const total = integrate(eco, j.res) / 3600e3;
              const eur_eco = total * Magnac.prix / 1e4;
              byId("eco_" + label).innerHTML =
                `${total.toFixed(3)} kWh
                &times 0.${Magnac.prix} € / kWh =
                ${eur_eco.toFixed(2)} € ! FIXME compte les hc`;

              Magnac.chtconso[label].data = {
                labels: hoursLabels(j.p2.length, j.res),
                datasets: [
                {
                  label: "Consommation/surplus (W)",
                  data: rotateArray(j.p2, j.ix),
                  pointStyle: false,
                  fill: {
                    target: 'origin',
                    above: "#ff000044",
                    below: "#00ff0044",
                  },
                },
                {
                  pointStyle: false,
                  label: "Chauffe-eau (W)",
                  data: rotateArray(p1, j.ix),
                }]
              };
              Magnac.chtconso[label].update("none");
            });
          }

          setInterval(updateOta,    1000);
          setInterval(updateWatts,  2000);
          setInterval(updateDimmer, 2000);
          setInterval(updateData, 180000, "24h");
          setInterval(updateData,   8000, "1h");
          setInterval(updateData,   2000, "15min");
          labels.forEach(updateData);
          
          // updateScreen();
          // setInterval(updateScreen,     2000);
        });
      })();
      </script>
      <title>RBR Magnac</title>
    </head>
    <body>
      <h1>Routeur solaire RBR Magnac</h1>
      <menu>
        <li><a href="/webserial">Web serial</a></li>
      </menu>
      <h2>À l'instant</h2>
      <ul class="data">
        <li>Dernier reboot : <b id="last_boot"></b></li>
        <li>Consommation totale : <b id="ptot"></b></li>
        <li>Consommation effective : <b id="p2"></b></li>
        <li>Consommation du chauffe-eau :
          <b id="p1"></b>
        </li>
      </ul>
      <h2>Consommation/surplus</h2>

      <div>0.<input type="number" id="ekWh" value="2874" />
      &nbsp;<label for="ekWh">€ / kWh</label></div>

      <h3>Consommation sur 24 heures</h3>
      <p>Cumulé : <b id="cumul_24h"></b></p>
      <p>Solaire vers chauffe-eau : <b id="eco_24h"></b></p>
      <div><canvas id="pltconso_24h"></canvas></div>

      <h3>Consommation sur une heure</h3>
      <p>Cumulé : <b id="cumul_1h"></b></p>
      <p>Solaire vers chauffe-eau : <b id="eco_1h"></b></p>
      <div><canvas id="pltconso_1h"></canvas></div>

      <h3>Consommation sur 15 minutes</h3>
      <p>Cumulé : <b id="cumul_15min"></b></p>
      <p>Solaire vers chauffe-eau : <b id="eco_15min"></b></p>
      <div><canvas id="pltconso_15min"></canvas></div>

      <div id="ota" class="disabled">
      <h2>Mise à jour OTA</h2>
        <b id="ota_text">Pas de mise à jour en cours.</b>
        <progress id="ota_progress" max="100">
          Pas de mise à jour en cours.
        </progress>
      </div>

      <div><canvas id="screen">Écran</canvas></div>
      </canvas></div>
    </body>
  </html>)%";
}

#endif /* INDEX_H */
