using System;
using System.ComponentModel;
using System.Linq.Expressions;
using System.Collections.Generic;

namespace System.Windows.Data
{
	static class INPCProperty
	{
		public static INPCProperty<TValue> Create<TValue> (Expression<Func<TValue>> expression, Func<PropertyChangedEventHandler> notifier)
		{
			return new INPCProperty<TValue>(expression, notifier);
		}
	}

	class INPCProperty<TValue>
	{
		TValue value;

		Func<PropertyChangedEventHandler> Notifier {
			get; set;
		}

		string PropertyName {
			get; set;
		}

		public TValue Value {
			get { return value; }
			set {
				this.value = value;
				// Get the current list of registered event handlers
				// then invoke them with the correct 'sender' and event args
				var h = Notifier ();
				if (h != null)
					h (Notifier.Target, new PropertyChangedEventArgs (PropertyName));
			}
		}

		public INPCProperty (Expression<Func<TValue>> expression, Func<PropertyChangedEventHandler> notifier)
		{
			if (expression.NodeType != ExpressionType.Lambda)
				throw new ArgumentException("Value must be a lamda expression", "expression");
			if (!(expression.Body is MemberExpression))
				throw new ArgumentException("The body of the expression must be a memberref", "expression");

			MemberExpression m = (MemberExpression) expression.Body;
			Notifier = notifier;
			PropertyName = m.Member.Name;
		}
	}
}
