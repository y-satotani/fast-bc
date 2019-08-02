研究進捗報告
================
里谷 佳紀
2019年8月1日

# 研究全体の目的

グラフと全頂点間距離と最短経路の数およびペア依存度が与えられたとき，グラフの辺が削除されたとする．
このときの全頂点間距離と最短経路の数とペア依存度を高速に更新する方法を開発する．
また，実験と理論の両方から既存の方法と比較することで，新方法の有用性を検証する．

# 前回打ち合わせ時に定めた短期目標

1.  修士論文の下書きの執筆
2.  追加実験

# 本日までの進捗状況

1.  章立てを検討し，論文に掲載する定理・補題を検討した(別紙参照)．
2.  ペア依存度を更新した頂点ペアと，媒介中心性が変化した頂点の数を記録する機能の実装が完了した．

<!-- end list -->

1)  実験を行った(別紙参照)．この結果は，理論的な解析結果と矛盾しない．
2)  いくつかの辺更新をまとめて行う状況を想定した実験を行った(別紙参照)．このような状況では，提案手法の方が更新の時間がかかることがわかった．
3)  道路ネットワークを対象とする実験のプログラムを実装している．