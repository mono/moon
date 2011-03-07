# [SecurityCritical] needed to execute code inside 'System.ServiceModel, Version=2.0.5.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35'.
# 1 methods needs to be decorated.

# overrides 'System.Runtime.Remoting.Messaging.IMessage System.Runtime.Remoting.Proxies.RealProxy::Invoke(System.Runtime.Remoting.Messaging.IMessage)'.
+SC-M: System.Runtime.Remoting.Messaging.IMessage System.ServiceModel.ClientRealProxy::Invoke(System.Runtime.Remoting.Messaging.IMessage)

