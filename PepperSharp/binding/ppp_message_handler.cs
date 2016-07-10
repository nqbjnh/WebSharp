/* Copyright (c) 2016 Xamarin. */

/* NOTE: this is auto-generated from IDL */
/* From ppp_message_handler.idl modified Thu May 12 07:00:02 2016. */

using System;
using System.Runtime.InteropServices;

namespace PepperSharp {

/**
 * @file
 * This file defines the <code>PPP_MessageHandler</code> interface that plugins
 * can implement and register using PPB_Messaging::RegisterMessageHandler in
 * order to handle messages sent from JavaScript via postMessage() or
 * postMessageAndAwaitResponse().
 */


/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * The <code>PPP_MessageHandler</code> interface is implemented by the plugin
 * if the plugin wants to receive messages from a thread other than the main
 * Pepper thread, or if the plugin wants to handle blocking messages which
 * JavaScript may send via postMessageAndAwaitResponse().
 *
 * This interface struct should not be returned by PPP_GetInterface; instead it
 * must be passed as a parameter to PPB_Messaging::RegisterMessageHandler.
 */
public static partial class PPPMessageHandler {
  [DllImport("PepperPlugin", EntryPoint = "PPP_MessageHandler_HandleMessage")]
  extern static void _HandleMessage ( PPInstance instance,
                                     ref IntPtr user_data,
                                      PPVar message);

  /**
   * Invoked as a result of JavaScript invoking postMessage() on the plugin's
   * DOM element.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance
   * of a module.
   * @param[in] user_data is the same pointer which was provided by a call to
   * RegisterMessageHandler().
   * @param[in] message A copy of the parameter that JavaScript provided to
   * postMessage().
   */
  public static void HandleMessage ( PPInstance instance,
                                    ref IntPtr user_data,
                                     PPVar message)
  {
  	 _HandleMessage (instance, ref user_data, message);
  }


  [DllImport("PepperPlugin",
             EntryPoint = "PPP_MessageHandler_HandleBlockingMessage")]
  extern static void _HandleBlockingMessage ( PPInstance instance,
                                             ref IntPtr user_data,
                                              PPVar message,
                                             out PPVar response);

  /**
   * Invoked as a result of JavaScript invoking postMessageAndAwaitResponse()
   * on the plugin's DOM element.
   *
   * NOTE: JavaScript execution is blocked during the duration of this call.
   * Hence, the plugin should respond as quickly as possible. For this reason,
   * blocking completion callbacks are disallowed while handling a blocking
   * message.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance
   * of a module.
   * @param[in] user_data is the same pointer which was provided by a call to
   * RegisterMessageHandler().
   * @param[in] message is a copy of the parameter that JavaScript provided
   * to postMessageAndAwaitResponse().
   * @param[out] response will be copied to a JavaScript object which is
   * returned as the result of postMessageAndAwaitResponse() to the invoking
   *
   */
  public static void HandleBlockingMessage ( PPInstance instance,
                                            ref IntPtr user_data,
                                             PPVar message,
                                            out PPVar response)
  {
  	 _HandleBlockingMessage (instance, ref user_data, message, out response);
  }


  [DllImport("PepperPlugin", EntryPoint = "PPP_MessageHandler_Destroy")]
  extern static void _Destroy ( PPInstance instance, ref IntPtr user_data);

  /**
   * Invoked when the handler object is no longer needed. After this, no more
   * calls will be made which pass this same value for <code>instance</code>
   * and <code>user_data</code>.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance
   * of a module.
   * @param[in] user_data is the same pointer which was provided by a call to
   * RegisterMessageHandler.
   */
  public static void Destroy ( PPInstance instance, ref IntPtr user_data)
  {
  	 _Destroy (instance, ref user_data);
  }


}
/**
 * @}
 */


}