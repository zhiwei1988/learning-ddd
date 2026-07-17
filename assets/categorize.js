/* Shared categorization exercise. Present an item (a stakeholder quote, a term,
 * a code smell) and ask which bucket it belongs to. Immediate feedback per item.
 *
 * Usage in a lesson:
 *
 *   <div id="sort"></div>
 *   <script src="../assets/categorize.js"></script>
 *   <script>
 *     renderCategorize(document.getElementById("sort"), {
 *       buckets: ["销售", "履约", "计费"],
 *       items: [{ text: "…", bucket: 0, explain: "…" }],
 *       perfect: "…", imperfect: "…",   // optional closing lines
 *     });
 *   </script>
 *
 * Buckets render in a fixed order, so avoid making position a clue: spread the
 * correct answers across buckets.
 *
 * `perfect` / `imperfect` name the skill this exercise drilled — always pass
 * them, since the default copy cannot know what was being practised.
 */
function renderCategorize(root, spec) {
  "use strict";
  const buckets = spec.buckets;
  const items = spec.items;
  const perfect = spec.perfect || "全对。";
  const imperfect = spec.imperfect || "回头看看答错那条的解释，那才是练习的重点。";
  let answered = 0;
  let correct = 0;

  const score = document.createElement("p");
  score.className = "quiz-score";

  items.forEach(function (item) {
    const box = document.createElement("div");
    box.className = "cat-item";

    const quote = document.createElement("p");
    quote.className = "quote";
    quote.textContent = item.text;
    box.appendChild(quote);

    const row = document.createElement("div");
    row.className = "buckets";

    const explain = document.createElement("p");
    explain.className = "explain";
    explain.textContent = item.explain;

    buckets.forEach(function (name, bi) {
      const btn = document.createElement("button");
      btn.type = "button";
      btn.textContent = name;
      btn.addEventListener("click", function () {
        row.querySelectorAll("button").forEach(function (b) { b.disabled = true; });
        if (bi === item.bucket) {
          btn.classList.add("correct");
          correct++;
        } else {
          btn.classList.add("wrong");
          row.querySelectorAll("button")[item.bucket].classList.add("correct");
        }
        explain.classList.add("show");
        answered++;
        if (answered === items.length) {
          score.textContent = "得分：" + correct + " / " + items.length + " — " +
            (correct === items.length ? perfect : imperfect);
          score.classList.add("show");
        }
      });
      row.appendChild(btn);
    });

    box.appendChild(row);
    box.appendChild(explain);
    root.appendChild(box);
  });

  root.appendChild(score);
}
