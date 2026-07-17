/* Shared quiz widget. Usage in a lesson:
 *
 *   <div id="quiz"></div>
 *   <script src="../assets/quiz.js"></script>
 *   <script>
 *     renderQuiz(document.getElementById("quiz"), [
 *       { stem: "…", options: ["…", "…"], answer: 0, explain: "…" },
 *     ], { perfect: "…", imperfect: "…" });   // opts optional
 *   </script>
 *
 * Immediate feedback on click; running score shown once all questions answered.
 * `opts.perfect` / `opts.imperfect` override the closing line — pass them when a
 * quiz is not an end-of-lesson check (e.g. a warm-up review), so the copy fits.
 */
function renderQuiz(root, questions, opts) {
  "use strict";
  opts = opts || {};
  const perfect = opts.perfect || "全对，可以进入下一课了。";
  const imperfect = opts.imperfect || "回头看看答错的题的解释，弄懂再走。";
  let answered = 0;
  let correct = 0;

  const score = document.createElement("p");
  score.className = "quiz-score";

  questions.forEach(function (q, qi) {
    const box = document.createElement("div");
    box.className = "quiz-q";

    const stem = document.createElement("p");
    stem.className = "stem";
    stem.textContent = (qi + 1) + ". " + q.stem;
    box.appendChild(stem);

    const opts = document.createElement("ul");
    opts.className = "opts";

    const explain = document.createElement("p");
    explain.className = "explain";
    explain.textContent = q.explain;

    q.options.forEach(function (text, oi) {
      const li = document.createElement("li");
      const btn = document.createElement("button");
      btn.type = "button";
      btn.textContent = text;
      btn.addEventListener("click", function () {
        opts.querySelectorAll("button").forEach(function (b) { b.disabled = true; });
        const isRight = oi === q.answer;
        if (isRight) {
          btn.classList.add("correct");
          correct++;
        } else {
          btn.classList.add("wrong");
          opts.querySelectorAll("button")[q.answer].classList.add("correct");
        }
        explain.classList.add("show");
        answered++;
        if (answered === questions.length) {
          score.textContent = "得分：" + correct + " / " + questions.length + " — " +
            (correct === questions.length ? perfect : imperfect);
          score.classList.add("show");
        }
      });
      li.appendChild(btn);
      opts.appendChild(li);
    });

    box.appendChild(opts);
    box.appendChild(explain);
    root.appendChild(box);
  });

  root.appendChild(score);
}
