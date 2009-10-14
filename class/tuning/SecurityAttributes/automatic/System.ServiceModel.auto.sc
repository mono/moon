# [SecurityCritical] needed to execute code inside 'System.ServiceModel, Version=2.0.5.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35'.
# 2 methods needs to be decorated.

# overrides 'System.Boolean System.Threading.WaitHandle::WaitOne(System.Int32,System.Boolean)'.
+SC-M: System.Boolean System.ServiceModel.ClientRuntimeChannel/DelegatingWaitHandle::WaitOne(System.Int32,System.Boolean)

# overrides 'System.Boolean System.Threading.WaitHandle::WaitOne(System.TimeSpan,System.Boolean)'.
+SC-M: System.Boolean System.ServiceModel.ClientRuntimeChannel/DelegatingWaitHandle::WaitOne(System.TimeSpan,System.Boolean)

