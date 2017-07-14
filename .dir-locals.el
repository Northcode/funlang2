((nil
  (eval . (let ((root (projectile-project-root)))
	    (setq-local flycheck-clang-include-path
			(list (file-truename (concat root "src/alb"))
			      (file-truename (concat root "src/inc"))))))))
