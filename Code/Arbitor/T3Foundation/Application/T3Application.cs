/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

namespace T3Foundation.Application
{
  public interface CT3Application
  {
    public string ApplicationName { get; set; }

    /// <summary>
    /// Is this application interacting with lower level languages? (e.g. C++, C-ABI)
    /// </summary>
    /// <value></value>
    public bool ManagedApplication { get; set; }
  }

  public partial class CT3WPFApplication : System.Windows.Application, CT3Application
  {
    public CT3WPFApplication()
    {

    }
    public bool ManagedApplication { get; set; }
    public string ApplicationName { get; set; }
  }
}
